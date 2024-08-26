# Git Stats

<!--toc:start-->
- [Git Stats](#git-stats)
  - [Features](#features)
  - [Requirements](#requirements)
  - [Installation](#installation)
      - [Debian Based Systems](#debian-based-systems)
      - [Other Systems](#other-systems)
<!--toc:end-->

This plugin is an integrated implementation of the [git-linestats-widget](https://github.com/BryanHaley/git-linestats-widget), designed to automate and streamline your git statistics display with [OBS](https://github.com/obsproject/obs-studio).

**NOTE:** This plugin is currently only available for Linux. Support for other operating systems may be added in the future.

## Features

- **Tracked and Untracked Files**: Specify which files to track and display.
- **Customizable Text**: Modify font and colors to suit your preferences.
- **Anti-Aliasing**: Ensure smooth and visually appealing text rendering.
- **Future Enhancements**: More features are planned and will be added soon. Feel free to open an issue with a freature request or take a look at the current [TODO](https://github.com/Love-Pengy/git-stats/issues/4) list.

## Requirements

- Linux system
- A monospaced [nerd font](https://github.com/ryanoasis/nerd-fonts) 

## Installation
#### Debian Based Systems
- Download git-stats-*-x86_64-linux-gnu.deb from the [Releases](https://github.com/Love-Pengy/git-stats/releases/) Page
- Install With ```apt install ./git-stats-*-x86_64-linux-gnu.deb```
    - This method will install the plugin in the usr directory of OBS

#### Other Systems

- Download git-stats-*-source.tar.xz from the [Releases](https://github.com/Love-Pengy/git-stats/releases/) page
- Extract with ```tar -xf git-stats-*-source.tar.xz```
- Change directory to the directory you just downloaded with ```cd git-stats-*-source```
- Run the following commands to build the project: 

    ```
    cmake --preset linux-x86_64 -G Ninja '-DQT_VERSION=6' '-DCMAKE_BUILD_TYPE=RelWithDebInfo' '-DCMAKE_INSTALL_PREFIX=/usr'
    cmake --build --preset linux-x86_64 --config RelWithDebInfo --parallel
    cmake --install build_x86_64 --prefix ./release/RelWithDebInfo
    mkdir -p ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats/bin/64bit
    cp ./release/RelWithDebInfo/lib/x86_64-linux-gnu/obs-plugins/git-stats.so ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats/bin/64bit    
    ```

- Copy the directory into your OBS plugin directory: ```cp -r ./release/RelWithDebInfo/share/obs/obs-plugins/git-stats ~/.config/obs-studio/plugins```

**NOTE:** If the ~/.config/obs-studio/plugins directory does not exist just create it

