# Half-Life Asset Manager

Half-Life Asset Manager is a model viewer for Half-Life 1. It can be used to view and edit Half-Life 1 Studio models (`.mdl`).

> **Note**
> This tool only supports Half-Life 1/GoldSource, not Half-Life 2/Source and Source 2!

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
