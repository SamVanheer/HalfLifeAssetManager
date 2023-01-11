# Changelog for Version 2

[< Back to CHANGELOG](CHANGELOG.md)

### Changes in V2.0.0

#### Installer changes

* Check if operating system is supported in installer [#227](https://github.com/SamVanheer/HalfLifeAssetManager/issues/227)

### Bug fixes

* Key input is now forwarded from the scene widget to the last object to have focus [#228](https://github.com/SamVanheer/HalfLifeAssetManager/issues/228)
* Removed minimum width settings from timeline sliders [#235](https://github.com/SamVanheer/HalfLifeAssetManager/issues/235)

### New features

* Added option to allow closing tabs using middle mouse button [#215](https://github.com/SamVanheer/HalfLifeAssetManager/issues/215)

### Project changes

* Now compiled with Visual Studio 2022
* Third party dependencies are now included using Git submodules instead of including the files in the repository itself or requiring developers to set up the dependencies manually. Dependencies are part of the generated build system and will be built as needed
* Development builds are automatically built by Github continuous integration and are available for 90 days as a downloadable artifact. See the [Actions](https://github.com/SamVanheer/HalfLifeAssetManager/actions) tab for automatic builds (based on Half-Life Unified SDK's CI configuration. Thanks Shepard and SmileyAG for helping make that configuration)
