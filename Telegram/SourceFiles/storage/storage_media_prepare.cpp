/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "storage/storage_media_prepare.h"

#include "data/data_document.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "history/history.h"
#include "history/history_item.h"
#include "history/history_item_helpers.h"
#include "data/data_msg_id.h"
#include "data/data_peer_id.h"
#include "data/data_document_media.h"
#include "data/data_photo_media.h"
#include "ui/image/image.h"
#include "data/data_photo.h"
#include "history/view/history_view_item_preview.h"
#include "ui/text/text_utilities.h"
#include "window/window_session_controller.h"
#include "editor/photo_editor_common.h"
#include "editor/scene/scene.h"
#include "editor/scene/scene_item_sticker.h"
#include "platform/platform_file_utilities.h"
#include "lang/lang_keys.h"
#include "storage/localimageloader.h"
#include "core/mime_type.h"
#include "ui/image/image_prepare.h"
#include "ui/chat/attach/attach_prepare.h"
#include "core/crash_reports.h"

#include <QtCore/QSemaphore>
#include <QtCore/QMimeData>

namespace Storage {
namespace {

using Ui::PreparedFileInformation;
using Ui::PreparedFile;
using Ui::PreparedList;

using Image = PreparedFileInformation::Image;

bool ValidPhotoForAlbum(
		const Image &image,
		const QString &mime) {
	Expects(!image.data.isNull());

	if (image.animated
		|| (!mime.isEmpty() && !mime.startsWith(u"image/"))) {
		return false;
	}
	const auto width = image.data.width();
	const auto height = image.data.height();
	return Ui::ValidateThumbDimensions(width, height);
}

bool ValidVideoForAlbum(const PreparedFileInformation::Video &video) {
	const auto width = video.thumbnail.width();
	const auto height = video.thumbnail.height();
	return Ui::ValidateThumbDimensions(width, height);
}

QSize PrepareShownDimensions(const QImage &preview, int sideLimit) {
	const auto result = preview.size();
	return (result.width() > sideLimit || result.height() > sideLimit)
		? result.scaled(sideLimit, sideLimit, Qt::KeepAspectRatio)
		: result;
}

void PrepareDetailsInParallel(PreparedList &result, int previewWidth) {
	Expects(result.files.size() <= Ui::MaxAlbumItems());

	if (result.files.empty()) {
		return;
	}
	const auto sideLimit = PhotoSideLimit(); // Get on main thread.
	QSemaphore semaphore;
	for (auto &file : result.files) {
		crl::async([=, &semaphore, &file] {
			PrepareDetails(file, previewWidth, sideLimit);
			semaphore.release();
		});
	}
	semaphore.acquire(result.files.size());
}

} // namespace

bool ValidatePhotoEditorMediaDragData(not_null<const QMimeData*> data) {
	const auto urls = Core::ReadMimeUrls(data);
	if (urls.size() > 1) {
		return false;
	} else if (data->hasImage()) {
		return true;
	}

	if (!urls.isEmpty()) {
		const auto url = urls.front();
		if (url.isLocalFile()) {
			using namespace Core;
			const auto file = Platform::File::UrlToLocal(url);
			const auto info = QFileInfo(file);
			return FileIsImage(file, MimeTypeForFile(info).name())
				&& QImageReader(file).canRead();
		}
	}

	return false;
}

bool ValidateEditMediaDragData(
		not_null<const QMimeData*> data,
		Ui::AlbumType albumType) {
	const auto urls = Core::ReadMimeUrls(data);
	if (urls.size() > 1) {
		return false;
	} else if (data->hasImage()) {
		return (albumType != Ui::AlbumType::Music);
	}

	if (albumType == Ui::AlbumType::PhotoVideo && !urls.isEmpty()) {
		const auto url = urls.front();
		if (url.isLocalFile()) {
			using namespace Core;
			const auto info = QFileInfo(Platform::File::UrlToLocal(url));
			return IsMimeAcceptedForPhotoVideoAlbum(MimeTypeForFile(info).name());
		}
	}

	return true;
}

MimeDataState ComputeMimeDataState(const QMimeData *data) {
	if (!data || data->hasFormat(u"application/x-td-forward"_q)) {
		return MimeDataState::None;
	}

	if (data->hasImage()) {
		return MimeDataState::Image;
	}

	const auto urls = Core::ReadMimeUrls(data);
	if (urls.isEmpty()) {
		return MimeDataState::None;
	}

	auto allAreSmallImages = true;
	auto allAreMedia = true;
	for (const auto &url : urls) {
		if (!url.isLocalFile()) {
			return MimeDataState::None;
		}
		const auto file = Platform::File::UrlToLocal(url);

		const auto info = QFileInfo(file);
		if (info.isDir()) {
			return (urls.size() == 1)
				? MimeDataState::Folder
				: MimeDataState::None;
		}

		using namespace Core;
		const auto filesize = info.size();
		if (filesize > kFileSizePremiumLimit) {
			return MimeDataState::None;
		//} else if (filesize > kFileSizeLimit) {
		//	return MimeDataState::PremiumFile;
		} else if (allAreSmallImages) {
			if (filesize > Images::kReadBytesLimit) {
				allAreSmallImages = false;
			} else {
				const auto mime = MimeTypeForFile(info).name();
				if (mime == u"image/gif"_q
					|| !FileIsImage(file, mime)
					|| !QImageReader(file).canRead()) {
					allAreSmallImages = false;
				}
			}
		}
		if (allAreMedia) {
			const auto type = DetectNameType(file);
			if (type != NameType::Image && type != NameType::Video) {
				allAreMedia = false;
			}
		}
	}
	return allAreSmallImages
		? MimeDataState::PhotoFiles
		: allAreMedia
		? MimeDataState::MediaFiles
		: (urls.size() > 1)
		? MimeDataState::FilesArchive
		: MimeDataState::Files;
}

PreparedList PrepareMediaList(
		const QList<QUrl> &files,
		int previewWidth,
		bool premium,
		Fn<void(const PreparedList &)> errorCallback) {
	auto locals = QStringList();
	locals.reserve(files.size());
	for (const auto &url : files) {
		if (!url.isLocalFile()) {
			auto errorResult = PreparedList(
				PreparedList::Error::NonLocalUrl,
				url.toDisplayString());
			if (!errorCallback) {
				return errorResult;
			}
			errorCallback(errorResult);
			continue;
		}
		locals.push_back(Platform::File::UrlToLocal(url));
	}
	return PrepareMediaList(
		locals,
		previewWidth,
		premium,
		std::move(errorCallback));
}

PreparedList PrepareMediaList(
		const QStringList &files,
		int previewWidth,
		bool premium,
		Fn<void(const PreparedList &)> errorCallback) {
	auto result = PreparedList();
	result.files.reserve(files.size());
	for (const auto &file : files) {
		const auto fileinfo = QFileInfo(file);
		const auto filesize = fileinfo.size();
		if (fileinfo.isDir()) {
			auto errorResult = PreparedList(
				PreparedList::Error::Directory,
				file);
			if (!errorCallback) {
				return errorResult;
			}
			errorCallback(errorResult);
			continue;
		} else if (!fileinfo.exists()
			|| !fileinfo.isFile()
			|| !fileinfo.isReadable()
			|| filesize <= 0) {
			auto errorResult = PreparedList(
				PreparedList::Error::EmptyFile,
				file);
			if (!errorCallback) {
				return errorResult;
			}
			errorCallback(errorResult);
			continue;
		} else if (filesize > kFileSizePremiumLimit
			|| (filesize > kFileSizeLimit && !premium)) {
			auto errorResult = PreparedList(
				PreparedList::Error::TooLargeFile,
				file);
			errorResult.files.emplace_back(file);
			errorResult.files.back().size = filesize;
			if (!errorCallback) {
				return errorResult;
			}
			errorCallback(errorResult);
			continue;
		}
		if (result.files.size() < Ui::MaxAlbumItems()) {
			result.files.emplace_back(file);
			result.files.back().size = filesize;
		} else {
			result.filesToProcess.emplace_back(file);
			result.filesToProcess.back().size = filesize;
		}
	}
	PrepareDetailsInParallel(result, previewWidth);
	return result;
}

PreparedList PrepareMediaFromImage(
		QImage &&image,
		QByteArray &&content,
		int previewWidth) {
	Expects(!image.isNull());

	auto result = PreparedList();
	auto file = PreparedFile(QString());
	file.content = content;
	if (file.content.isEmpty()) {
		file.information = std::make_unique<PreparedFileInformation>();
		const auto animated = false;
		FileLoadTask::FillImageInformation(
			std::move(image),
			animated,
			file.information);
	}
	result.files.push_back(std::move(file));
	PrepareDetailsInParallel(result, previewWidth);
	return result;
}

std::optional<PreparedList> PreparedFileFromFilesDialog(
		FileDialog::OpenResult &&result,
		Fn<bool(const Ui::PreparedList&)> checkResult,
		Fn<void(tr::phrase<>)> errorCallback,
		int previewWidth,
		bool premium) {
	if (result.paths.isEmpty() && result.remoteContent.isEmpty()) {
		return std::nullopt;
	}

	auto list = result.remoteContent.isEmpty()
		? PrepareMediaList(result.paths, previewWidth, premium)
		: PrepareMediaFromImage(
			QImage(),
			std::move(result.remoteContent),
			previewWidth);
	if (list.error != PreparedList::Error::None) {
		errorCallback(tr::lng_send_media_invalid_files);
		return std::nullopt;
	} else if (!checkResult(list)) {
		return std::nullopt;
	} else {
		return list;
	}
}

void PrepareDetails(PreparedFile &file, int previewWidth, int sideLimit) {
	if (!file.path.isEmpty()) {
		file.information = FileLoadTask::ReadMediaInformation(
			file.path,
			QByteArray(),
			Core::MimeTypeForFile(QFileInfo(file.path)).name());
	} else if (!file.content.isEmpty()) {
		file.information = FileLoadTask::ReadMediaInformation(
			QString(),
			file.content,
			Core::MimeTypeForData(file.content).name());
	} else {
		Assert(file.information != nullptr);
	}

	using Video = PreparedFileInformation::Video;
	using Song = PreparedFileInformation::Song;
	if (const auto image = std::get_if<Ui::PreparedFileInformation::Image>(
			&file.information->media)) {
		Assert(!image->data.isNull());
		if (ValidPhotoForAlbum(*image, file.information->filemime)) {
			UpdateImageDetails(file, previewWidth, sideLimit);
			file.type = PreparedFile::Type::Photo;
		} else {
			file.originalDimensions = image->data.size();
			if (image->animated) {
				file.type = PreparedFile::Type::None;
			}
		}
	} else if (const auto video = std::get_if<Ui::PreparedFileInformation::Video>(
			&file.information->media)) {
		if (ValidVideoForAlbum(*video)) {
			video->modifications.gif = !video->hasAudio;
			UpdateVideoDetails(file, previewWidth, sideLimit);
			file.type = PreparedFile::Type::Video;
		}
	} else if (v::is<Song>(file.information->media)) {
		file.type = PreparedFile::Type::Music;
	}
}

VideoDetails ComputeVideoDetails(
		const QImage &thumbnail,
		const Editor::PhotoModifications &geometry,
		int previewWidth,
		int sideLimit) {
	if (thumbnail.isNull()) {
		return {};
	}
	// The thumbnail stays raw, the modifications are applied on read.
	auto preview = geometry
		? Editor::ImageModified(base::duplicate(thumbnail), geometry)
		: base::duplicate(thumbnail);
	Assert(!preview.isNull());
	auto result = VideoDetails{
		.originalDimensions = preview.size(),
		.shownDimensions = PrepareShownDimensions(preview, sideLimit),
	};
	// Blur whichever of the source and the result has fewer pixels. Blurring
	// a full resolution frame allocates and works by the source pixel count,
	// and washes the blur out in proportion to how much is scaled away, so a
	// 4K video ended up with a far sharper preview than a small one.
	const auto width = previewWidth * style::DevicePixelRatio();
	auto opaque = Images::Opaque(std::move(preview));
	result.preview = (opaque.width() > width)
		? Images::Blur(
			opaque.scaledToWidth(width, Qt::SmoothTransformation))
		: Images::Blur(std::move(opaque)).scaledToWidth(
			width,
			Qt::SmoothTransformation);
	Assert(!result.preview.isNull());
	result.preview.setDevicePixelRatio(style::DevicePixelRatio());
	return result;
}

void ApplyVideoDetails(PreparedFile &file, VideoDetails &&details) {
	if (details.preview.isNull()) {
		return;
	}
	file.originalDimensions = details.originalDimensions;
	file.shownDimensions = details.shownDimensions;
	file.preview = std::move(details.preview);
}

void UpdateVideoDetails(
		PreparedFile &file,
		int previewWidth,
		int sideLimit) {
	using Video = PreparedFileInformation::Video;
	const auto video = std::get_if<Video>(&file.information->media);
	if (!video) {
		return;
	}
	ApplyVideoDetails(
		file,
		ComputeVideoDetails(
			video->thumbnail,
			video->modifications.geometry,
			previewWidth,
			sideLimit));
}

void UpdateImageDetails(
		PreparedFile &file,
		int previewWidth,
		int sideLimit) {
	const auto image = std::get_if<Ui::PreparedFileInformation::Image>(&file.information->media);
	if (!image) {
		return;
	}
	Assert(!image->data.isNull());
	auto preview = image->modifications
		? Editor::ImageModified(image->data, image->modifications)
		: image->data;
	Assert(!preview.isNull());
	file.originalDimensions = preview.size();
	file.shownDimensions = PrepareShownDimensions(preview, sideLimit);
	const auto toWidth = std::min(
		previewWidth,
		style::ConvertScale(preview.width())
	) * style::DevicePixelRatio();
	auto scaled = preview.scaledToWidth(
		toWidth,
		Qt::SmoothTransformation);
	if (scaled.isNull()) {
		CrashReports::SetAnnotation("Info", QString("%1x%2:%3*%4->%5;%6x%7"
		).arg(preview.width()).arg(preview.height()
		).arg(previewWidth).arg(style::DevicePixelRatio()
		).arg(toWidth
		).arg(scaled.width()).arg(scaled.height()));
		Unexpected("Scaled is null.");
	}
	Assert(!scaled.isNull());
	file.preview = Images::Opaque(std::move(scaled));
	Assert(!file.preview.isNull());
	file.preview.setDevicePixelRatio(style::DevicePixelRatio());
}

bool ApplyModifications(PreparedList &list, bool composeAnimated) {
	auto applied = false;
	const auto apply = [&](PreparedFile &file, QSize strictSize = {}) {
		const auto image = std::get_if<Ui::PreparedFileInformation::Image>(&file.information->media);
		const auto guard = gsl::finally([&] {
			if (!image || strictSize.isEmpty()) {
				return;
			}
			applied = true;
			file.path = QString();
			file.content = QByteArray();
			image->data = image->data.scaled(
				strictSize,
				Qt::IgnoreAspectRatio,
				Qt::SmoothTransformation);
		});
		if (!image || !image->modifications) {
			return;
		}
		applied = true;
		file.path = QString();
		file.content = QByteArray();
		const auto &scene = image->modifications.paint;
		if (composeAnimated && scene && scene->hasAnimatedItems()) {
			auto job = Editor::ComposeAnimatedJob(
				image->data,
				image->modifications);
			const auto animated = ranges::any_of(
				job.overlay,
				[](const Media::Encode::Layer &layer) {
					const auto entity
						= std::get_if<Media::Encode::AnimatedEntity>(
							&layer);
					return entity && !entity->bytes.isEmpty();
				});
			if (animated) {
				file.animationJob = std::make_shared<Media::Encode::Job>(
					std::move(job));
			}
		}
		image->data = Editor::ImageModified(
			std::move(image->data),
			image->modifications);
		if (file.animationJob) {
			auto &ids = file.animationJob->attachedStickerIds;
			for (const auto &item : scene->items()) {
				if (item->isVisible()
					&& (item->type() == Editor::ItemSticker::Type)) {
					const auto sticker
						= static_cast<Editor::ItemSticker*>(item.get());
					ids.push_back(sticker->sticker()->id);
				}
			}
			image->modifications = Editor::PhotoModifications();
		}
	};
	for (auto &file : list.files) {
		apply(file);
		if (const auto cover = file.videoCover.get()) {
			const auto video = file.information
				? std::get_if<Ui::PreparedFileInformation::Video>(
					&file.information->media)
				: nullptr;
			apply(*cover, video ? video->thumbnail.size() : QSize());
		}
	}
	return applied;
}


// Deserializes media references from clipboard and populates metadata.
// Extract media metadata from clipboard using custom MIME format.
// Uses Qt_5_1 for robust serialization across potential client instances.
Ui::PreparedList ReadMediaRef(not_null<const QMimeData*> data) {
	if (!data->hasFormat(u"application/x-td-media-ref"_q)) {
		return {};
	}
	const auto refData = data->data(u"application/x-td-media-ref"_q);
	QDataStream stream(refData);
	stream.setVersion(QDataStream::Qt_5_1);
	uint64 sessionId;
	int32 count;
	stream >> sessionId >> count;
	if (stream.status() != QDataStream::Ok || count <= 0) {
		return {};
	}
	auto session = SessionByUniqueId(sessionId);
	if (!session) {
		return {};
	}
	bool withCaption = false;
	std::vector<HistoryItem*> items;
	for (int i = 0; i < count; ++i) {
	        uint64 peerId;
	        int32 msgId;
	        stream >> peerId >> msgId;
	        const auto msgIdFull = FullMsgId(PeerId(peerId), msgId);
	        if (const auto item = session->data().message(msgIdFull)) {
	                items.push_back(item);
	        }
	}
	stream >> withCaption;	auto list = Ui::PreparedList();
	using Image = Ui::PreparedFileInformation::Image;
	using Video = Ui::PreparedFileInformation::Video;
	for (const auto item : items) {
		const auto media = item->media();
		if (!media) continue;

		const auto isVideo = media->document()
			&& (media->document()->isVideoFile()
				|| media->document()->isAnimation()
				|| media->document()->isGifv());

		auto file = Ui::PreparedFile(QString());
		file.information = std::make_unique<Ui::PreparedFileInformation>();

		QByteArray itemRefData;
		QDataStream itemStream(&itemRefData, QIODevice::WriteOnly);
		itemStream.setVersion(QDataStream::Qt_5_1);
		itemStream << sessionId
			<< uint64(item->history()->peer->id.value)
			<< int32(item->id.bare);
		file.referenceData = itemRefData;
		file.isReference = true;

		auto preview = QImage();
		if (auto doc = media->document()) {
			if (isVideo) {
				file.type = Ui::PreparedFile::Type::Video;
			} else if (doc->isVoiceMessage() || doc->isSong()) {
				file.type = Ui::PreparedFile::Type::Music;
			} else {
				file.type = Ui::PreparedFile::Type::File;
			}
			file.size = doc->size;
			file.displayName = doc->filename();
			file.information->filemime = doc->mimeString();
			if (isVideo) {
				file.originalDimensions = doc->dimensions;
			}

			if (file.type == Ui::PreparedFile::Type::Music) {
				auto song = Ui::PreparedFileInformation::Song();
				if (const auto data = doc->song()) {
					song.title = data->title;
					song.performer = data->performer;
				}
				song.duration = doc->duration();
				file.information->media = std::move(song);
			} else if (file.type == Ui::PreparedFile::Type::Video) {
				auto video = Ui::PreparedFileInformation::Video();
				video.isGifv = doc->isAnimation() || doc->isGifv();
				video.duration = doc->duration();
				file.information->media = std::move(video);
			}

			const auto mediaView = doc->activeMediaView();
			if (mediaView && (file.type == Ui::PreparedFile::Type::Video)) {
				if (const auto img = mediaView->thumbnail()) {
					preview = img->original();
				}
			}
		} else if (auto photo = media->photo()) {
			file.type = Ui::PreparedFile::Type::Photo;
			auto image = Ui::PreparedFileInformation::Image();
			const auto mediaView = photo->activeMediaView();
			if (mediaView) {
				if (const auto img = mediaView->image(Data::PhotoSize::Large)) {
					preview = img->original();
				}
			}
			if (const auto size = photo->size(Data::PhotoSize::Large)) {
				file.originalDimensions = *size;
			}
			image.data = preview;
			file.information->media = std::move(image);
		} else {
			continue;
		}

		if (!preview.isNull()) {
			file.preview = preview;
			file.shownDimensions = preview.size();
			file.originalDimensions = preview.size();
			const auto info = file.information.get();
			if (const auto image = std::get_if<Image>(&info->media)) {
				image->data = preview;
			} else if (const auto video = std::get_if<Video>(&info->media)) {
				video->thumbnail = preview;
			}
		}

		if (withCaption) {
			const auto text = item->originalText();
			file.caption.text = text.text;
			file.caption.tags = TextUtilities::ConvertEntitiesToTextTags(
				text.entities);
		}
		list.files.push_back(std::move(file));
	}
	return list;
}

} // namespace Storage
