#!/bin/bash
set -euo pipefail

# Essensial logo improvements
# Can't replace assets due to github censorship

cd Telegram/Resources/art

# is 1024:1024
wget -O Cumgram.png https://files.catbox.moe/zmzhcf.png

# -vf scale=1024:1024
ffmpeg -y -i Cumgram.png -c copy icon512@2x.png
ffmpeg -y -i Cumgram.png -c copy icon_round512@2x.png
ffmpeg -y -i Cumgram.png -c copy icon_green.png

ffmpeg -y -i Cumgram.png -vf scale=512:512 icon512.png
ffmpeg -y -i Cumgram.png -vf scale=512:512 icon256@2x.png

ffmpeg -y -i Cumgram.png -vf scale=256:256 logo_256_no_margin.png
ffmpeg -y -i Cumgram.png -vf scale=256:256 logo_256.png
ffmpeg -y -i Cumgram.png -vf scale=256:256 iconbig_green.png
ffmpeg -y -i Cumgram.png -vf scale=256:256 icon256.png
ffmpeg -y -i Cumgram.png -vf scale=256:256 icon128@2x.png

ffmpeg -y -i Cumgram.png -vf scale=128:128 icon128.png
ffmpeg -y -i Cumgram.png -vf scale=128:128 icon64@2x.png

ffmpeg -y -i Cumgram.png -vf scale=96:96 icon48@2x.png

ffmpeg -y -i Cumgram.png -vf scale=64:64 icon64.png
ffmpeg -y -i Cumgram.png -vf scale=64:64 icon32@2x.png

ffmpeg -y -i Cumgram.png -vf scale=48:48 icon256.ico
ffmpeg -y -i Cumgram.png -vf scale=48:48 icon48.png

ffmpeg -y -i Cumgram.png -vf scale=32:32 icon32.png
ffmpeg -y -i Cumgram.png -vf scale=32:32 icon16@2x.png

ffmpeg -y -i Cumgram.png -vf scale=16:16 icon16.png

rm Cumgram.png

# Ignore improved pics
cd -
cat .gitignore-tracked | tr '\n' '\0' | xargs -0 git update-index --skip-worktree

# Unignore
# cat .gitignore-tracked | tr '\n' '\0' | xargs -0 git update-index --no-skip-worktree
