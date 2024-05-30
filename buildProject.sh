#!/bin/sh

rm -rf ./release/ 
rm -rf ~/.config/obs-studio/plugins/git-stats/
./.github/scripts/build-linux --skip-deps
mkdir ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats/bin
mkdir ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats/bin/64bit
cp ./release/RelWithDebInfo/lib/x86_64-linux-gnu/obs-plugins/git-stats.so ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats/bin/64bit
mkdir ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats/data
mv ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats/locale  ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats/data/
mv ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats ~/.config/obs-studio/plugins



