# Half-Life Asset Manager

Half-Life Asset Manager is a model viewer for Half-Life 1. It can be used to view and edit Half-Life 1 Studio models (`.mdl`).

> **Note**
> This tool only supports Half-Life 1/GoldSource, not Half-Life 2/Source and Source 2!

## Features

* Opens Half-life 1 Studiomodel version 10 models
* Recognizes Quake 1, Xash3D, Source 1 and Nexon model formats (all `.mdl`) and supports forwarding to a compatible model viewer for them
* Dock-based interface allows interface elements to be moved and hidden
* Tabbed interface allows multiple models to be opened, able to open **ALL** models in a mod without slowing down
* Able to edit parts of the model's data (e.g. import and export textures, change sequence properties, hitboxes, etc)
* Able to switch between three cameras: Arc Ball, Free Look and First Person
* Real-time colormap preview (top and bottom color in the Multiplayer tab in-game)
* Graphics options: texture filtering and resizing, multi-sample anti-aliasing, vertical sync
* Audio playback for sequence events
* Game configuration wizard automatically detects installed games and mods with automatic configuration switching based on the location of a model, including on-the-fly detection of loose files if they use a mod's directory structure

## Minimum requirements

* Operating system: Windows 7 or newer
* OpenGL version: 2.1 or newer

> **Note**
> [Half-Life Model Viewer 2](https://github.com/SamVanheer/HalfLifeModelViewer2) works on Windows XP and newer. If you cannot run Asset Manager then that tool may be a better option.

## Download

See the releases for a download link: https://github.com/SamVanheer/HalfLifeAssetManager/releases

## Development builds

Development builds can be downloaded from the Github Actions runs: https://github.com/SamVanheer/HalfLifeAssetManager/actions

These are work-in-progress builds with known issues. Refrain from using these for anything other than testing individual features and bug fixes.

It is recommended to use portable mode (see below) to avoid corrupting the configuration file.

You will need to install the latest Visual C++ x86 redistributable yourself to run these builds: https://aka.ms/vs/17/release/vc_redist.x86.exe

## Building

Requirements:
* CMake 3.25 or newer
* Visual Studio 2022 or newer (Windows) or GCC 11 (Linux)
* Qt 5.15.5 (Windows: download 32 bit MSVC 2019 version using Qt maintenance tool. Linux: install package `qtbase5-dev`)

Clone the repository using Git. Make sure to clone submodules as well (`--recurse-submodules` on the command line).

Use CMake to generate project files for your platform. When specifying how to set up the project make sure to choose "toolchain" and point it to `<repository root>/vcpkg/scripts/buildsystems/vcpkg.cmake`.

Run configure. You will need to specify the variable `Qt5_DIR`. This should point to the path `<Qt install directory>/5.15.2/msvc2019/lib/cmake/Qt5` (Windows) or `/usr/lib/x86_64-linux-gnu/cmake/Qt5` (Linux). The exact path may differ depending on your system.

Point `CMAKE_INSTALL_PREFIX` to `<repository root>/installer/packages/SamVanheer.HalfLifeAssetManager/data`.

Generate the project files and use them to build the project.

Run the install target to deploy required files to the installer directory.

To create the installer you will need to put the program executable and all required Qt libraries in the bin directory, and place the Visual Studio x86 redistributable in the redist directory.
The PDF manual generated from [HalfLifeAssetManagerManual.md](../docs/manual/HalfLifeAssetManagerManual.md) should be placed in the manual directory.

See this for more information on packaging Qt applications: https://doc.qt.io/qt-5/windows-deployment.html

## Installation

The installer will take care of most of the installation process. Simply run the installer, change the install location if desired, choose which file extensions to associate the program with, and the installer will do the rest.

If you already have file associations set up for one or more of the file types supported by Half-Life Asset Manager then you will have to manually change the association in the [control panel](https://www.thewindowsclub.com/change-file-associations-windows).

The installer will also install the Visual Studio re-distributable. If it has already been installed then no changes will be made.

## Uninstalling

To uninstall Half-Life Asset Manager, simply run the program `maintenancetool.exe` located in the Half-Life Asset Manager install directory. Choose the option "Uninstall all components" to uninstall the program.

## Updating

To update Half-Life Asset Manager first uninstall the current version and then install the new version. User configuration files are not removed by the uninstaller so you will not lose any settings.

## Options

Asset Manager provides a number of options that allow for customization and enables it to provide a more complete representation of the assets it has.

### Game configurations

Game configurations allow Asset Manager to access game content located in both the main game directory, as well as SteamPipe directories. It also supports mod directories. See [Game configurations](/docs/game-configurations.md) for more information.

### Portable Mode

Launching Half-Life Asset Manager with the command line parameter `--portable` launches it in portable mode. In this mode the program configuration file is stored next to the executable, which makes it easier to run the program from a removable storage device or with multiple user accounts.

## Differences between Model Viewer and Asset Manager

Asset Manager is based on Model Viewer and as such has the same functionality. However Asset Manager has a completely rebuilt user interface designed using Qt instead of wxWidgets, and incorporates many improvements and new features.

## Changelog

See [Changelog](/docs/CHANGELOG.md)

## Debug log output

See [Configuring Log File Output](/docs/configuring-log-output.md)

## Third party libraries

There are several third party libraries used by Half-Life Asset Manager.

Most of these are acquired using [vcpkg](https://vcpkg.io). You can find a list of these dependencies here: https://github.com/SamVanheer/HalfLifeAssetManager/blob/dev/vcpkg.json

Additionally Qt 5.15.2 (cross platform GUI framework) is also used and has to be installed using Qt's maintenance tool. The maintenance tool is part of the Qt open source release, which you can download here: https://www.qt.io/download

# License

See [LICENSE.md](LICENSE.md)
