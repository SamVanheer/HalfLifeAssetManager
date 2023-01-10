# Game Configurations

[< Back to Half-Life Asset Manager](/README.md)

Asset Manager supports the use of game configurations. These allow the tools to access game and mod specific content.

Since the SteamPipe update GoldSource games use a new directory structure for addon, hd and downloaded content. This configuration enables the toolchain to access game content in these directories.

## Available options

A single game configuration currently contains the following options:

Option | Description
------ | -----------
Name | Each configuration has a name. This name is unique and identifies the configuration.
Game Executable | Path to the game executable.
Base game directory | The base path points to the root game directory, for example "common\Half-Life".
Base game directory | The absolute path to the game directory. Typically this is "path/to/Half-Life/valve".
Mod directory | The absolute path to the mod directory. This can be left empty if the configuration is meant to be used for the game itself.
