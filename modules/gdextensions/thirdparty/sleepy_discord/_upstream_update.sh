#!/bin/bash

set -e

trap "{ if [ -d '_sleepydiscord' ]; then rm -rf _sleepydiscord; fi; exit 255; }" SIGINT SIGTERM ERR EXIT


rm -rf _sleepydiscord
git clone --depth=1 --recursive --no-single-branch https://github.com/yourWaifu/sleepy-discord.git _sleepydiscord
rm -rf README.md LICENSE.md src inc
mv _sleepydiscord/README.md .
mv _sleepydiscord/LICENSE.md .
mv _sleepydiscord/include inc
mv _sleepydiscord/sleepy_discord src

rm -rf _sleepydiscord
