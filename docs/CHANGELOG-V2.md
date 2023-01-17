# Changelog for Version 2

[< Back to CHANGELOG](CHANGELOG.md)

## Changes in V2.0.0

### Github repository changes

* Renamed the Github repository to `HalfLifeAssetManager`
* Moved Half-Life Model Viewer 2 source code and documentation to the [HalfLifeModelViewer2](https://github.com/SamVanheer/HalfLifeModelViewer2) repository
* Added asset source files for the app icon
* Converted the wiki documentation to Markdown files in the repository

### Installer changes

* The installer now checks if the operating system is supported to prevent installation on platforms where the program will crash on start
* Added license covering use of the program and third party libraries
* Added the option to create a Start menu entry and desktop shortcut during installation

### Bug fixes

* Re-implemented hardware accelerated texture view. This fixes bugs with the texture not updating when imported or when the colormap is changed
* Eliminated redundant texture upload calls. This improves performance
* Fixed texture not updating when the texture name is changed to one matching a color remapping name
* Filenames passed on the command line are now reformed if passed without quotes. This matches the behavior of other model viewers and allows models to be opened through JACK
* Key input is now forwarded from the scene widget to the last object to have focus
* Improved floating point spin box editing behavior to prevent annoying auto-fill of zeroes
* Wave sound files are no longer converted from 32 bit float to 8 or 16 bit PCM. This should improve the audio quality for high quality sounds
* Switching assets while fullscreen mode is active now switches the asset in the fullscreen window as well
* The `File Browser` no longer tries to open directories as if they were files
* Loading an empty model that has no T.mdl file is no longer invalid. The model will be assumed to have no such file
* Replaced StudioModel length check with validation to prevent models with extra data from crashing the program
* Removed minimum width settings from timeline sliders to allow them to shrink more
* Fixed sequence FPS using scientific notation if the value is too large
* A sequence FPS of 0 now sets duration to `Infinite`
* Fixed exported textures becoming corrupted due to use-after-free of image pixel data
* Fixed refreshing models resetting the skin value
* Fixed Attachment panel using the wrong index to change the attachment bone
* Fixed program always opening as maximized even when closed in windowed state
* Fixed changes made to colors applying even if the Options dialog was closed with the Cancel button
* Fixed portable mode not always placing the configuration file next to the executable
* Fixed shadows being rotated towards lighting instead of according to lighting
* Fixed crash when enabling mipmaps on computers that don't support OpenGL 3.0
* Fixed centering on large models centering on the wrong position
* Fixed bounding box, clipping box & player hitbox not always rendering on top of the model
* Fixed Dump Model Info not setting name field

### New features

#### General

* Experimental Linux support (Thanks Shepard for helping to fix compiler errors)
* Many optimizations to reduce memory usage and improve performance
* The program log file is now stored next to the configuration file
* Added log message for sound playback. This shows the full sound file path and is useful for debugging
* Configuration file data has been changed to use consistent case style. This means existing settings will not be recognized. Many changes have been made that render existing settings obsolete or unusable so it will be necessary to reconfigure the program on first use

#### Menus and related functionality

* Multiple assets can be loaded at the same time using the Load dialog. Using `CTRL+A` it is possible to load *all* models in a directory at the same time. Note that this may take a few seconds
* Opening an asset that is already opened will now refresh the existing asset tab
* Added the option to limit to only one asset open at a time (matches original model viewer behavior)
* Assets pause when they are not the active asset to save CPU time (improves performance)
* PS2 (`*.dol`) models are no longer renamed on load, instead the extension is changed on save to the correct one
* When saving a file with a new name where another asset has the file with that name already opened the user is asked what to do (discard other asset or cancel)
* Added `Close All` action to the `File` menu
* Added dropdown menu showing list of all loaded assets for quick navigation
* Added progress dialogs to asset load and close to give the user the opportunity to cancel if the process takes too long
* Optimized asset load and close to dramatically reduce the time required. This also speeds up program shutdown with many assets opened
* Asset filenames are removed from the `Recent Files` menu if the file fails to load for any reason
* Optimized some undo commands to halve their memory usage
* Simplified some undo commands
* Switching back and forth between borderless fullscreen and bordered now remembers the bordered window state to improve window state restoration
* Redesigned the `File List` panel to be a more useful `File Browser`. The `File Browser` can quickly switch directories based on the available game configurations and can filter files based not on their extension but by checking their contents. Opening a directory is a bit slower as a result but provides more accurate results and allows fast navigation between files. Combining the `Limit to one asset at a time` feature with the arrow and Enter keys allows switching between models just like the `Previous` and `Next` actions in Half-Life Model Viewer 2. Users can navigate through directories like they would in Windows Explorer
* Added the `Messages` panel. This panel shows messages from the application. When an error occurs the panel is automatically opened to show the error. It is also possible to enable debug message output to get more information about the program's behavior
* The `Play Sounds` and `Pitch *= Framerate` (renamed to `Framerate affects Pitch`) checkboxes are now global settings in the new `Audio` menu
* Moved `Power Of 2 Textures` and filter settings to the new `Video` menu, removed settings from Options dialog and Textures panel
* Disabled the `Power Of 2 Textures` option by default. This option will be enabled automatically if the program determines that your computer does not support Non-Power Of 2 textures
* Added Multisample Anti-Aliasing option in the `Video` menu. Changing this setting will re-create the 3D window which may cause a slight flicker
* Added the option to take transparent screenshots. This can be enabled by checking the `Transparent Screenshots` option in the `Video` menu. Note that the background color will still affect transparent objects. To counter this set the background color to black before taking screenshots
* The StudioModel dock widgets are now saved and loaded to retain their docking position, visible and active states. The `Reset Dock Widgets` action in the `Asset` menu resets these to their original state
* Added option to hide the Controls Bar, Timeline and all Edit Controls (switches to maximized 3D window). These settings are remembered and restored
* The `Save View` and `Restore View` actions now operate globally across all assets, allowing the current camera and camera state to be saved in one asset and restored in another
* Reworked the `Model Info` panel into the `Show QC Data` dialog. This dialog shows the model's data as (pseudo-) QC data
* Added Half-Life Asset Manager manual. The manual explains what each UI element does and how they behave. There is also information on features like color remapping
* Added action to the `Help` menu to open the manual

#### Options Dialog

* Added button to the Options dialog to open the configuration file directory in Windows Explorer
* Added option to allow closing tabs using middle mouse button
* Added vertical sync option
* Added support for changing a color's alpha value in the Color options page for colors that support it
* Added the option to specify the filename for Quake 1 Model Viewer, Source 1 Model Viewer and Xash Model Viewer to delegate the loading of models from these engines to
* Added the event Ids `1011`, `1012` and `1013` from Opposing Force as valid sound events
* Added the option to specify which event Ids correspond to sound events in addition to built-in event Ids
* Reworked game configurations to use Hammer-like configuration
* Added game configurations auto-detect wizard to automatically find and add all games and mods in a game installation
* Each asset now uses its own filesystem configured for it based on the asset file location to automatically locate related files correctly (e.g. sounds referenced by events)
	* This works even if the asset isn't located in a game directory; if the file is located in a game asset directory like `models` any directories related to it will be used to locate files. A common case is a downloaded model with related sounds which will play correctly.
	* This also supports the use of fallback directories, so a Condition Zero model can reference a sound file from Counter-Strike and play it as it would in-game
	* This also detects and uses language-specific directories
	* Game directories containing additional content like HD or Addon content are searched only if the user enables this behavior

#### StudioModel interface

* The StudioModel renderer is now shared between assets, dramatically reducing memory usage (nearly 3MB per asset)
* Loading a Studio model that has sequence group and/or external texture files now shows information in the Messages panel indicating that those files have been merged into the main file. Saving the file will save the merged file
* Objects in the 3D scene are now sorted back to front to combat graphical issues involving transparent objects
* Reworked the StudioModel user interface to use a single shared instance between all assets
* Reworked user interface to reduce overall size to allow the window to be made smaller
	* Reworked how some 3D vector edit widgets are presented to dramatically reduce the size of the user interface and improve the editing experience
* Reworked the 3D window to use a single shared instance for all 3D rendering. This reduces the amount of graphics resources allocated for use with windows. Note that this stops models from rendering in the normal window while the fullscreen window is open
* Added option to switch between Perspective and Orthographic projection in Scene view for the Arc Ball camera. Orthographic mode tries to maintain a good zoom to match the equivalent Perspective zoom, but this behavior is limited and does not work well with unusual window aspect ratios
* Reworked Cameras panel to make contents for each camera consistent
* Camera field of view settings are now remembered
* The Ground texture directory is now remembered
* Added the `Enable Texture` option to the Scene->Ground panel. Enabling this without a texture loaded now uses an auto-generated grid texture
* Added the option to draw transparent faces on hitboxes. This can be enabled by changing the `HitboxFace` color alpha value to make these faces visible
* The `Show Axes` option now renders axes on top of the model
* Added the option to choose the aspect ratio used by the `Show Guidelines` option
* Added `Show Off-screen Areas` option to mask portions of the 3D window that are invisible in-game when using the selected aspect ratio
* Added global light color selector, changed sky light color name to `SkyLight`
* Added the option to change the sky light's ambient ahd shade lighting values (normally calculated by the engine, not possible here)
* Made the Sky Light panel change orientation when docked vertically
* Re-implemented keyboard and mouse shortcuts for light vector control
* It is now possible to edit a sequence's looping flag framerate, activity, activity weight and linear movement vector. Changing the linear movement vector also updates the linear movement flags (LX, LY and LZ)
* Added support for custom activities using the `ACT_<number>` syntax supported by the studiomdl compiler. This fixes `tentacle2.mdl` not showing the right activities
* Split the Body Parts panel into Body Parts and Bone Controllers
* The last path used in the Import and Export Texture options is now saved
* Texture widths are now required to be multiples of 4 to match the engine's expectations. Textures will be resized on import if needed and the user will be warned about this
* Removed the `bmp` extension from the editable texture name control (extension is required to be there) and blocked setting the name to an empty string or having whitespace at the start or end
* The UV map is drawn using OpenGL again as it did in Half-Life Model Viewer 2. This results in sharper lines removing the need for the UV line width setting. Note that the Export UV Map dialog does not use OpenGL to draw the UV map. This will be changed in Half-Life Asset Manager 3.0.0

### Removed features

* Removed the ability to change bone parents. This feature was too buggy and could not be implemented in a way that was useful
* Removed bone controller option from the Bones panel. This functionality already exists in the Bone Controller panel and only complicated things
* Removed bone flags, attachment type and event type user interface controls (never used in-game, never set by compiler)
* Removed redundant log messages
* Removed the `Windows` submenu; the File Browser is now accessible directly in the `View` menu

### Project changes

* Now compiled with Visual Studio 2022
* All third party dependencies except Qt5 are now included using vcpkg instead of including the files in the repository itself or requiring developers to set up the dependencies manually (Qt5 requires only the Qt5_DIR variable to be set; the rest are discovered automatically)
* Development builds are automatically built by Github continuous integration and are available for 90 days as a downloadable artifact. See the [Actions](https://github.com/SamVanheer/HalfLifeAssetManager/actions) tab for automatic builds (based on Half-Life Unified SDK's CI configuration. Thanks Shepard and SmileyAG for helping make that configuration)
	* 64 bit Linux is also built, but only for continuous integration testing. The artifacts produced by these builds do not run on their own
* Audio loading is now handled by libnyquist instead of AudioFile
* Project info is regenerated at build time instead of at CMake generate time
* Removed GLEW library dependency
* Redesigned the graphics scene architecture to make scenes lightweight objects
* The offscreen OpenGL context is used to load graphics resources to eliminate the need to wait for the 3D window to initialize
