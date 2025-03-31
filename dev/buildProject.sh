#!/bin/bash

# Move to git-stats dir
dirname=`dirname -- "$0"`
dirname+="/.."
cd $dirname 

# Remove installed .deb packages
sudo apt-get purge git-stats
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug --preset ubuntu-x86_64 --fresh 
cmake --build --preset ubuntu-x86_64
mkdir -p build_x86_64/git-stats
mkdir -p build_x86_64/git-stats/bin
mkdir -p build_x86_64/git-stats/data
mkdir -p build_x86_64/git-stats/bin/64bit
mkdir -p build_x86_64/git-stats/data/locale
cp build_x86_64/git-stats.so build_x86_64/git-stats/bin/64bit 
rm -rf ~/.config/obs-studio/plugins/git-stats
mv build_x86_64/git-stats ~/.config/obs-studio/plugins
