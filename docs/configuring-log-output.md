# Configuring Log Output

[< Back to Half-Life Asset Manager](/README.md)

Half-Life Asset Manager can output log information to a file. To enable this add the command line parameter `--log-to-file` to the command line.

Create a desktop shortcut to add this when launching the program directly.

Log output will be written to a text file called `HLAM-Log.txt` located in the same directory as the program configuration file.

When running the program using its default configuration this will be in `%AppData%/Roaming/Half-Life Asset Manager` (Windows).
When running in portable mode this will be in the `bin` directory.

You can open this directory by opening the options dialog and clicking `Open Configuration Directory`.

The file is cleared on startup.

### Configuring logging categories

Qt uses logging categories to filter log output. To control which categories are enabled, create a file called `qtlogging.ini` in `Half-Life Asset Manager/bin` and add the following:
```
[Rules]
```

Now add logging rules below this section, for example:
```
[Rules]
qt.qpa.gl=true
```

This will enable the logging category for OpenGL.