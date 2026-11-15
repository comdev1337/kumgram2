# kumgram2

Meme fork of [Telegram Desktop](https://github.com/telegramdesktop/tdesktop), kept rebased on upstream `dev`.

## Features

| Commit | Change |
| --- | --- |
| [`8216664a60`](https://github.com/comdev1337/kumgram2/commit/8216664a60) | Playback speed options: 0.1×–14×, step 0.25× (upstream caps at 0.5×–2.5×) |
| [`356cf774cc`](https://github.com/comdev1337/kumgram2/commit/356cf774cc) | Don't pause music when a video's audio plays |
| [`f8f0382c14`](https://github.com/comdev1337/kumgram2/commit/f8f0382c14) | Improve account-adding flow |
| [`80eeec9d18`](https://github.com/comdev1337/kumgram2/commit/80eeec9d18) | Essential script to generate improved app icons/logos |
| [`aaddc23902`](https://github.com/comdev1337/kumgram2/commit/aaddc23902) | Disable MSVC generating gigabytes of `.pdb` garbage on every build. Prevent 100% CPU usage. |
| [`deb2fa7791`](https://github.com/comdev1337/kumgram2/commit/deb2fa7791) | Right-click the speed gear to toggle speedup |
| [`5402cabac0`](https://github.com/comdev1337/kumgram2/commit/5402cabac0) | Shift + mouse wheel to set playback speed |
| [`f62a97ca7a`](https://github.com/comdev1337/kumgram2/commit/f62a97ca7a) | Helper script to parse video API JSON |
| [`85f8f5f730`](https://github.com/comdev1337/kumgram2/commit/85f8f5f730) | Allow forwarding fake channels (fake and gay client-side restriction) |
| [`34c6695f44`](https://github.com/comdev1337/kumgram2/commit/34c6695f44) | Experimental: copy/paste files by reference, combine into groups, no reupload required |
| [`c829f435ab`](https://github.com/comdev1337/kumgram2/commit/c829f435ab) | Support 8K video upload and playback |
| [`3190ee209f`](https://github.com/comdev1337/kumgram2/commit/3190ee209f) | Follow links inside timestamps in blockquotes |
| [`138499ce56`](https://github.com/comdev1337/kumgram2/commit/138499ce56) | Embed `.webm`, `.mkv` and `.m4v` video files |
| [`49df737826`](https://github.com/comdev1337/kumgram2/commit/49df737826) | Script to prepare dependency build garbage |
| [`17f3350573`](https://github.com/comdev1337/kumgram2/commit/17f3350573) | Paste Excel selections as article tables |
| [`31dbe0b372`](https://github.com/comdev1337/kumgram2/commit/31dbe0b372) | Allow saving and copying restricted channel posts (fake and gay client-side restriction) |
| [`316b3d4bb6`](https://github.com/comdev1337/kumgram2/commit/316b3d4bb6) | Kill malicious client-side transcoding of "unsupported" video (`.webm`, `.mkv`, `.m4v`) on upload |
| [`d17376300b`](https://github.com/comdev1337/kumgram2/commit/d17376300b) | Ignore absolutely pointless `supports_streaming` video flag.<br>Allows playing (streaming) videos uploaded by retarded bots, whose creators do not know what [`documentAttributeVideo`](https://core.telegram.org/constructor/documentAttributeVideo) is. |

## Before building

1. Improve logos

```bash
./logo_improvement.sh
```
2. [Windows MSVC] If you do not want to stare at 5+ GiB (!) of `.pdb` garbage generated on each build

```bash
./DISABLE_PDB_GARBAGE.sh
```

---

# [Telegram Desktop][telegram_desktop] – Official Messenger

This is the complete source code and the build instructions for the official [Telegram][telegram] messenger desktop client, based on the [Telegram API][telegram_api] and the [MTProto][telegram_proto] secure protocol.

[![Version](https://badge.fury.io/gh/telegramdesktop%2Ftdesktop.svg)](https://github.com/telegramdesktop/tdesktop/releases)
[![Build Status](https://github.com/telegramdesktop/tdesktop/workflows/Windows./badge.svg)](https://github.com/telegramdesktop/tdesktop/actions)
[![Build Status](https://github.com/telegramdesktop/tdesktop/workflows/MacOS./badge.svg)](https://github.com/telegramdesktop/tdesktop/actions)
[![Build Status](https://github.com/telegramdesktop/tdesktop/workflows/Linux./badge.svg)](https://github.com/telegramdesktop/tdesktop/actions)
[![Built with Depot](https://img.shields.io/badge/Built%20with-Depot.dev-46A75A)](https://depot.dev)

[![Preview of Telegram Desktop][preview_image]][preview_image_url]

The source code is published under GPLv3 with OpenSSL exception, the license is available [here][license].

## Supported systems

The latest version is available for

* [Windows 7 and above (64 bit)](https://telegram.org/dl/desktop/win64) ([portable](https://telegram.org/dl/desktop/win64_portable))
* [Windows 7 and above (32 bit)](https://telegram.org/dl/desktop/win) ([portable](https://telegram.org/dl/desktop/win_portable))
* [macOS 10.13 and above](https://telegram.org/dl/desktop/mac)
* [Linux static build for 64 bit](https://telegram.org/dl/desktop/linux)
* [Snap](https://snapcraft.io/telegram-desktop)
* [Flatpak](https://flathub.org/apps/details/org.telegram.desktop)

## Old system versions

Version **4.9.9** was the last that supports older systems

* [macOS 10.12](https://updates.tdesktop.com/tmac/tsetup.4.9.9.dmg)
* [Linux with glibc < 2.28 static build](https://updates.tdesktop.com/tlinux/tsetup.4.9.9.tar.xz)

Version **2.4.4** was the last that supports older systems

* [OS X 10.10 and 10.11](https://updates.tdesktop.com/tosx/tsetup-osx.2.4.4.dmg)
* [Linux static build for 32 bit](https://updates.tdesktop.com/tlinux32/tsetup32.2.4.4.tar.xz)

Version **1.8.15** was the last that supports older systems

* [Windows XP and Vista](https://updates.tdesktop.com/tsetup/tsetup.1.8.15.exe) ([portable](https://updates.tdesktop.com/tsetup/tportable.1.8.15.zip))
* [OS X 10.8 and 10.9](https://updates.tdesktop.com/tmac/tsetup.1.8.15.dmg)
* [OS X 10.6 and 10.7](https://updates.tdesktop.com/tmac32/tsetup32.1.8.15.dmg)

## Third-party

* Qt 6 ([LGPL](http://doc.qt.io/qt-6/lgpl.html)) slightly patched
* OpenSSL 3.2.1 ([Apache License 2.0](https://openssl-library.org/source/license/apache-license-2.0.txt))
* WebRTC ([New BSD License](https://github.com/desktop-app/tg_owt/blob/master/LICENSE))
* zlib ([zlib License](http://www.zlib.net/zlib_license.html))
* LZMA SDK 9.20 ([public domain](http://www.7-zip.org/sdk.html))
* liblzma ([public domain](http://tukaani.org/xz/))
* Google Breakpad ([License](https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE))
* Google Crashpad ([Apache License 2.0](https://chromium.googlesource.com/crashpad/crashpad/+/master/LICENSE))
* GYP ([BSD License](https://github.com/bnoordhuis/gyp/blob/master/LICENSE))
* Ninja ([Apache License 2.0](https://github.com/ninja-build/ninja/blob/master/COPYING))
* OpenAL Soft ([LGPL](https://github.com/kcat/openal-soft/blob/master/COPYING))
* Opus codec ([BSD License](http://www.opus-codec.org/license/))
* FFmpeg ([LGPL](https://www.ffmpeg.org/legal.html))
* Guideline Support Library ([MIT License](https://github.com/Microsoft/GSL/blob/master/LICENSE))
* Range-v3 ([Boost License](https://github.com/ericniebler/range-v3/blob/master/LICENSE.txt))
* Open Sans font ([Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html))
* Vazirmatn font ([SIL Open Font License 1.1](https://github.com/rastikerdar/vazirmatn/blob/master/OFL.txt))
* Emoji alpha codes ([MIT License](https://github.com/emojione/emojione/blob/master/extras/alpha-codes/LICENSE.md))
* xxHash ([BSD License](https://github.com/Cyan4973/xxHash/blob/dev/LICENSE))
* QR Code generator ([MIT License](https://github.com/nayuki/QR-Code-generator#license))
* CMake ([New BSD License](https://github.com/Kitware/CMake/blob/master/Copyright.txt))
* Hunspell ([LGPL](https://github.com/hunspell/hunspell/blob/master/COPYING.LESSER))
* zxcvbn-c ([MIT License](https://github.com/tsyrogit/zxcvbn-c/blob/master/LICENSE.txt))
* twidget ([BSD 3-Clause License](https://github.com/TelegramMessenger/twidget/blob/main/LICENSE))
* litehtml ([BSD 3-Clause License](https://github.com/litehtml/litehtml/blob/master/LICENSE))
* Gumbo ([Apache License 2.0](https://github.com/google/gumbo-parser/blob/master/COPYING))
* QuickJS-ng ([MIT License](https://github.com/quickjs-ng/quickjs/blob/master/LICENSE))
* FrequencyWords by Hermit Dave ([Creative Commons Attribution-ShareAlike 4.0](https://creativecommons.org/licenses/by-sa/4.0/))

## Build instructions

* [Windows][win]
* [macOS][mac]
* [GNU/Linux using Docker][linux]

[//]: # (LINKS)
[telegram]: https://telegram.org
[telegram_desktop]: https://desktop.telegram.org
[telegram_api]: https://core.telegram.org
[telegram_proto]: https://core.telegram.org/mtproto
[license]: LICENSE
[win]: docs/building-win.md
[mac]: docs/building-mac.md
[linux]: docs/building-linux.md
[preview_image]: https://github.com/telegramdesktop/tdesktop/blob/dev/docs/assets/preview.png "Preview of Telegram Desktop"
[preview_image_url]: https://raw.githubusercontent.com/telegramdesktop/tdesktop/dev/docs/assets/preview.png

## Thanks to

<a href="https://depot.dev">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://depot.dev/assets/brand/1693758816/depot-logo-horizontal-on-dark.svg">
    <source media="(prefers-color-scheme: light)" srcset="https://depot.dev/assets/brand/1693758816/depot-logo-horizontal-on-light.svg">
    <img alt="Depot" src="https://depot.dev/assets/brand/1693758816/depot-logo-horizontal-on-light.svg" width="150">
  </picture>
</a>

CI infrastructure sponsored by [Depot](https://depot.dev) — fast GitHub Actions runners.

