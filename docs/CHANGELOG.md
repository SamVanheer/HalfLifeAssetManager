# Changelog

[< Back to Half-Life Asset Manager](/README.md)

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

### Changes in V1.3.0

#### Bug Fixes

* Fixed FPS counter not updating when Scene view is inactive [#174](https://github.com/SamVanheer/HalfLifeAssetManager/issues/174)
* Fixed nothing being rendered if there are no body parts in the model [#220](https://github.com/SamVanheer/HalfLifeAssetManager/issues/220)
* Fixed refreshing models leaking GPU memory [#206](https://github.com/SamVanheer/HalfLifeAssetManager/issues/206)
* Fixed a memory leak that caused the studiomodel entity to not be freed when the asset is closed
* Fixed drag'n-drop no longer working when an asset is open [#202](https://github.com/SamVanheer/HalfLifeAssetManager/issues/202)
* Fixed mirror on axis not always mirroring on the correct axis [#199](https://github.com/SamVanheer/HalfLifeAssetManager/issues/199)
* Fixed Show Normals drawing normals behind the model if certain other Show checkboxes are enabled [#173](https://github.com/SamVanheer/HalfLifeAssetManager/issues/173)

#### UI Changes

* Used consistent label format (removal of colons) in UI [#166](https://github.com/SamVanheer/HalfLifeAssetManager/issues/166)
* Used consistent spacing in UI [#167](https://github.com/SamVanheer/HalfLifeAssetManager/issues/167)
* Redesigned studio model UI to use dock widgets [#168](https://github.com/SamVanheer/HalfLifeAssetManager/issues/168)
    * The Textures panel now draws textures on a separate Texture view that can be toggled using a separate tab bar next to the FPS and drawn polygons counters. By default opening the Textures panel while it is in tabbed dock mode will also switch to the Texture view (and closing the panel will switch back to the Scene view), but this can be disabled in the Options dialog
* Added 4 pixels of margin on the left and right of dock panels to improve readability of text on the edges of panels
* Streamlined UI widgets using SetFixedSize constraint (this reduces widget size when used as dock panels) [#165](https://github.com/SamVanheer/HalfLifeAssetManager/issues/165)
* Moved Model Flags back to its own panel and made it hidden by default since it's rarely used [#170](https://github.com/SamVanheer/HalfLifeAssetManager/issues/170)
* Added Skeleton pose (the skeleton as defined in a modeling program) as a viewable pose through a combo box on the main view bar. This disables the Sequences panel because sequence settings do not affect the model in this pose [#175](https://github.com/SamVanheer/HalfLifeAssetManager/issues/175)
* Model Data panel changes: [#169](https://github.com/SamVanheer/HalfLifeAssetManager/issues/169)
    * Flip Normals is now an `Asset` menu action.
    * The Origin and Scale edit widgets are now part of a new panel called `Transformation`. Hidden by default, can be opened either through the `Asset->Panels` menu, by right clicking in a dock area or by using the keyboard shortcut `CTRL+M`. This panel lets you move the model's contents and scale parts of the model. The behavior of this panel differs a bit from the Model Data version in that it doesn't try to retain information about previous transformation operations. The `Move` mode always treats the input as the amount to move the model, rather than the absolute offset to move the model to. A checkbox has been added that auto-resets the values entered on application, and the values are also reset when the panel is closed.
    * The remaining Model Data edit widgets have been placed side by side for easier use. When docked in the left or right dock areas, or floated, the panel will lay itself out vertically to minimize its width.
* Added more checkboxes to Scale mode [#186](https://github.com/SamVanheer/HalfLifeAssetManager/issues/186)
    * Separated Scale Meshes into Scale Meshes, Scale Hitboxes and Scale Sequence BBoxes
    * Added Scale Eye Position and Scale Attachments
* Added the option to rotate the entire model [#155](https://github.com/SamVanheer/HalfLifeAssetManager/issues/155)
    * This is part of the new `Transformation` panel. It will rotate root bones and change their rotation values. This may not work perfectly depending on the model (models with multiple root bones tend to have problems rotating), if you encounter problems it's best to modify the model in a modeling program instead.
    * Note that this rotates the skeleton which is often posed differently than the actual animations, so use the new Skeleton pose feature to better preview changes.
* Added a label to show the number of root bones in the model to the Sequences panel [#177](https://github.com/SamVanheer/HalfLifeAssetManager/issues/177)
* The program will now remember which screen it was last opened on and will open on that screen [#125](https://github.com/SamVanheer/HalfLifeAssetManager/issues/125)
* Added the option to pause sequence playback when clicking on the timeline [#124](https://github.com/SamVanheer/HalfLifeAssetManager/issues/124)
* Moved existing model object position manipulation widgets (Center On World Origin, Align On Ground) to the new Scene panel [#179](https://github.com/SamVanheer/HalfLifeAssetManager/issues/179)
* Moved existing ground & background widgets (Show, Load/Unload, etc) to the Scene panel [#180](https://github.com/SamVanheer/HalfLifeAssetManager/issues/180)
* The ground and background are now automatically shown when selecting a texture [#181](https://github.com/SamVanheer/HalfLifeAssetManager/issues/181)
* Added ground origin edit widget to allow changing the ground position in the world [#108](https://github.com/SamVanheer/HalfLifeAssetManager/issues/108)
    * Note: mirror on ground does not work properly if the ground has been moved up or down. This is slated for a fix in HLAM 2.0.0, see [#182](https://github.com/SamVanheer/HalfLifeAssetManager/issues/182)
* The object that the model is attached to can now be moved around in the 3D scene (this does not change model data) [#184](https://github.com/SamVanheer/HalfLifeAssetManager/issues/184)
* PS2 `.dol` Studiomodel files are now loaded using a separate extension in the file extension list, and are now renamed on load to `.mdl`. The UI no longer incorrectly claims to be able to save models as `.dol`. Using `CTRL+S` to save will no longer overwrite the `.dol` file with a regular `.mdl` file as a result [#183](https://github.com/SamVanheer/HalfLifeAssetManager/issues/183)
* Made OpenGL version check non-fatal, made it into a one off warning instead [#189](https://github.com/SamVanheer/HalfLifeAssetManager/issues/189)
* Added actions to place camera on positive or negative X/Y/Z axis [#191](https://github.com/SamVanheer/HalfLifeAssetManager/issues/191)

#### Project Changes

* Added build configuration info to About dialog [#217](https://github.com/SamVanheer/HalfLifeAssetManager/issues/217)
* Use QMessageBox::about to show About dialog [#218](https://github.com/SamVanheer/HalfLifeAssetManager/issues/218)
* Added About Qt action to the Help menu [#219](https://github.com/SamVanheer/HalfLifeAssetManager/issues/219)
* Reworked uses of `byte` type to use either `std::byte` or `std::uint8_t` [#212](https://github.com/SamVanheer/HalfLifeAssetManager/issues/212)
* Made the list of entities use dynamic allocation (this reduced memory usage by several dozen kilobytes) [#210](https://github.com/SamVanheer/HalfLifeAssetManager/issues/210)
*  Improved entity creation & destruction logic [#211](https://github.com/SamVanheer/HalfLifeAssetManager/issues/211)
* Force desktop OpenGL to be used at all times to prevent Qt from trying to use ANGLE (which fails to create OpenGL contexts) [#189](https://github.com/SamVanheer/HalfLifeAssetManager/issues/189)
* Lowered minimum required OpenGL version to 2.1 [#189](https://github.com/SamVanheer/HalfLifeAssetManager/issues/189)
* Added command line parameter `--log-to-file` to log to a file along with log output to help diagnose problems [#201](https://github.com/SamVanheer/HalfLifeAssetManager/issues/201), [#208](https://github.com/SamVanheer/HalfLifeAssetManager/issues/208)
    * Reworked existing logging code to use Qt's logging system. Sound system, sprite and studio model renderer log output is now sent to the log file [#209](https://github.com/SamVanheer/HalfLifeAssetManager/issues/209)
* Reworked matrix calculations to be consistent [#194](https://github.com/SamVanheer/HalfLifeAssetManager/issues/194)
* Improved application cleanup on shutdown
* Removed redundant exception types
* Removed obsolete code
* Cleaned up source code directory structure a bit
* Miscellaneous refactoring and cleanup

### Changes in V1.2.0

#### Bug Fixes

* Clamped texture remap color indices to valid range to prevent crashes [#152](https://github.com/SamVanheer/HalfLifeAssetManager/issues/152)
* Fixed highlighted bones, attachments and hitboxes not aligning properly in first person view [#151](https://github.com/SamVanheer/HalfLifeAssetManager/issues/151)
* Hardened HLAM against empty model data (no sequences, textures, etc) which could cause crashes. The user interface now disables sections that have no data to show [#158](https://github.com/SamVanheer/HalfLifeAssetManager/issues/158)
* Fixed faulty check in the Timeline that caused unnecessary frame spinner updates every tick. This caused framerate drops of around 33% (180 to 120 during testing), could be more or less depending on the computer [#159](https://github.com/SamVanheer/HalfLifeAssetManager/issues/159)
* Fixed animations stuttering at high tick rates [#161](https://github.com/SamVanheer/HalfLifeAssetManager/issues/161)
* Reworked asset loading so the main asset file is opened only once [#162](https://github.com/SamVanheer/HalfLifeAssetManager/issues/162)
* Ensured asset files are only opened if they aren't opened in any other programs [#163](https://github.com/SamVanheer/HalfLifeAssetManager/issues/163)

#### UI Changes

* Added CTRL+W shortcut to close the current tab along with Close action in the File menu [#149](https://github.com/SamVanheer/HalfLifeAssetManager/issues/149)
* Added button to Model Data panel to flip normals [#147](https://github.com/SamVanheer/HalfLifeAssetManager/issues/147)
* Re-implemented F5 to refresh feature from HLMV 2 along with Refresh action in the Tools menu, now also restores user interface state when possible (resets to appropriate defaults when possible) [#68](https://github.com/SamVanheer/HalfLifeAssetManager/issues/68)
* Finished re-implementation of HLMV 2.9 features in HLAM [#92](https://github.com/SamVanheer/HalfLifeAssetManager/issues/92)

#### Project Changes

* Removed EnTT library & reworked usages [#156](https://github.com/SamVanheer/HalfLifeAssetManager/issues/156)
* Updated Qt version from 5.14.2 to 5.15.2 Long Term Support
* Reworked camera operator management [#157](https://github.com/SamVanheer/HalfLifeAssetManager/issues/157)
* Reworked CS style blending to work when set from anywhere in the program [#160](https://github.com/SamVanheer/HalfLifeAssetManager/issues/160)

### Changes in V1.1.0

#### Bug Fixes

* Disabled blending after additive textured mesh has been drawn to fix axes lines from being drawn additively
* Texture effects are now only applied when not drawing meshes as wireframe to ensure wireframe overlay is properly drawn
* Fixed attachments and hitboxes panels causing invalid access if there are no attachments/hitboxes and a bone is renamed
* Fixed StudioModelEntity::SetMouth not checking if a mouth controller actually exists
* Fixed flat shade not working properly when shadows are drawn (thanks Malortie)
* Fixed attachments panel not drawing correct attachment sometimes
* Ensured entity is not null before checking sequence (thanks Malortie)
* Mirrored ground scrolling direction when model is mirrored (thanks Malortie)
* Enabled backface culling when drawing floor quad used to limit mirrored model draw region (prevents drawing the mirrored model underneath the floor)
* Fixed mirrored models having broken chrome (Thanks Malortie)
* Saved & restored state of cull face when drawing mirrored floor to prevent model from drawing on top of player hitbox model
* Fixed parts of model drawing texture when render mode is not texture shaded and shadows are enabled (thanks Malortie)
* Fixed player hitbox having inconsistent blending (thanks Malortie)
* Fixed framerate slider rounding values incorrectly and making it impossible to reach minimum and maximum values (thanks Malortie)

#### UI Changes

* Added dialog shown when a non-existent file is opened (e.g. deleted asset listed in recent files list)
* Added Git information to the About dialog
* Reworked Sequences and Model Data panels UI to limit maximum width
* Added controls to the Bones panel to view and change the bone controllers attached to a bone
    * Improved UI functionality for changing which bone and axis a bone controller is attached to. Changing the settings now automatically detaches any controllers attached to target bone and axis
* Implemented Bounding Box and Clipping Box modification support
    * Implemented Show Bounding Box (brownish box) and Show Clipping Box (orangeish box) checkboxes
* Implemented option to switch to Counter-Strike style sequence animation blending
* Avoid unnecessary framerate updates during timeline setup

#### Studiomodel file format

* Studio model data is now converted to a format that can be easily edited (note: data not part of the studiomodel format and data in the format not used by vanilla Half-Life is not saved and will be lost on save)
* Sequence groups and textures are now automatically merged into the main model file on save
* Imported textures can now have dimensions that differ from the original texture. ST coordinates will be rescaled to match the new size, but this may result in slight offsets in the coordinates in some cases (note: undo/redo requires a fair amount of memory to support this (4 bytes per vertex in every mesh that uses the texture), can easily get out of control)
* Implemented support for addition and removal of sequence events
    * Events are now sorted when saved to ensure that the game's event code finds newly added events properly

#### Backend changes

* Renamed Camera view matrix to model matrix
* Cached Camera view matrix instead of recalculating it every frame (thanks Malortie)
* Cached Camera projection matrix instead of recalculating it every frame (thanks Malortie)
* Ensured Camera matrices are initialized to identity matrices on creation

### Changes in V1.0.0

#### Project wide changes

* Replaced wxWidgets GUI library with Qt
* Compiled using the Visual Studio 2019 toolset
* Cleaned up main gitignore file
* Added missing GLM files
* Multiple assets can now be loaded at the same time and are accessible through tabs
* Generalized asset management to support file types other than Studiomodel version 10 (work in progress)
* Undo/redo support
* Standardized source code conventions (work in progress):
    * Header file extensions: .hpp
    * `#pragma once` instead of include guards
    * Simplified naming scheme for types and variables:
        * Type names, constants, global and public member variables: Pascal case
        * Private and protected member variables: lower camel case, prefixed with underscore
    * No mutable global variables (work in progress)
* Removed cvar system
* The maximum framerate is a bit lower than version 2 due to limitations on how often the window can be updated. With more updates on the graphics code stable 100 FPS should be perfectly possible, currently the framerate is still fairly good but may be affected by larger models, different hardware, etc, due to the use of OpenGL immediate mode. More work will be done to improve this. Currently the 3D window renders at the vertical refresh rate
* Implemented drag 'n drop support for multiple files. A single drop action that contains multiple local file URLs will open each file in its own tab, provided the file format is supported
* Added File List dock panel. This panel lists the files in the active game configuration's game directory and allows opening of files in the viewer. The root directory defaults to the current game configuration, and can be changed by browsing to another directory
* Pressing F11 while the fullscreen window is active toggles between fullscreen and maximized
* The last path used by load & save dialog actions is saved to allow the program to remember the last used directory
* Reworked max fps to be tick rate instead. This setting does not affect frame rate, only the frequency at which objects update
* Added keyboard shortcuts for load, save as and exit
* Added --portable command line parameter to use local settings file. The settings file will be created next to the executable just like Half-Life Model Viewer does
* Asset file paths are cleaned before loading to ensure consistency when loading from different sources. This prevents the same file from being added to the recent files list multiple times
* An option to disable audio playback has been added. If the loading of audio libraries causes problems that prevent you from opening the Options dialog, it is possible to manually disable loading by editing the settings file in `FOLDERID_RoamingAppData/Half-Life Asset Manager/Half-Life Asset Manager.ini` and adding this:
    ```
    [audio]
    EnableAudioPlayback=false
    ```
    Where `FOLDERID_RoamingAppData` is the location of the `Roaming` directory, usually located at `Users/AccountName/AppData/Roaming`.

* Fixed most issues preventing compilation on Linux (thanks to Shepard for providing these):
    * Added missing header includes, use correct header filename case
    * Added missing typename keywords
    * Made pure virtual destructors regular virtual
    * Use strncasecmp instead of _strnicmp on non-Windows platforms
    * Removed forced 32 bit compile flag for Linux, use correct exclude libs argument

* Implemented custom style sheet support. See the [Qt documentation](https://doc.qt.io/qt-5/stylesheet.html) for information on how to use style sheets. This allows you to implement dark mode and other styles

#### Studiomodel

* Fixed the coordinate system so that all positions and angles work on the correct axes. The coordinate system as used by model viewer from its first incarnation up until now rotated the world instead of applying a view matrix, which resulted in incorrect calculations elsewhere
    * Fixed the studio model renderer inverting the view right vector when calculating chrome. This still worked because the coordinate system was broken
* Added the option to autodetect viewmodels. When enabled, any model whose filename starts with `v_` will default to the first person camera
* Added ground texture scrolling. The texture scrolls at the ground speed set by the animation, both in the X and Y axes depending on the values in the linear movement vector
* Moved lighting controls from CTRL + left mouse button drag to its own panel. 2 dials allow changing the X and Y axis angles to adjust the lighting vector
* Reworked the compiler and decompiler frontends to use a single dialog design and to add the command line options for vanilla studiomdl as convenience settings. The executable paths can be configured from this dialog as well, eliminating the need to go to the Options dialog first.
* Accounted for studiomodel texture file ending with lowercase T (for case sensitive filesystems) (Thanks Shepard)
* The studio model header length member is now validated to prevent corrupt models from loading and crashing the program
* Increased maximum vertices limit to 65535 from 2048 to support models that use increased limits
* Added eye position edit control

##### Cameras

* The Arc Ball camera (previously called "Free View") now orbits around the target position instead of the world origin. This position is calculated based on the size of the model and centers the camera on it. As before, holding Shift while dragging with the left mouse button moves the camera, but now the target position is moved instead of the camera position itself being moved
* A new camera has been added: Free Look. This camera starts at the same position as the Arc Ball camera but moves freely in the world. Dragging with the left mouse button turns the camera, dragging with the right mouse button moves the camera forward and backward, allowing you to move around and view the model from angles that the Arc Ball camera cannot reach. Holding Shift while dragging with the left mouse button moves the camera left, right, up and down
* The First Person camera works as before. The field of view setting can now be found in the Cameras panel
* Added a mouse sensitivity slider in the range 1 to 20 and defaulting to 5, which is the original movement speed
* Made mouse 3 button (back button) enable SHIFT dragging behavior
* The camera can be moved forward/backward using the scroll wheel, with configurable speed setting in options dialog

#### Options Dialog

* Navigation uses a tree with categories and category entries. This allows grouping related options together without putting them all on the same page
* Moved "Power Of 2 Textures" option from the General page to the Textures Panel

#### Menus

* Added "Unload Background Texture" command
* Added Assets menu containing asset type-specific actions
* Added "Previous Camera" and "Next Camera" actions to cycle between cameras. These also have keyboard shortcuts (CTRL+U and CTRL+I respectively)
 
#### Control panels

* The main control bar, which contained camera, fullscreen and light vector controls has been removed. FPS and drawn polygons info is now displayed above the control panel area. Camera and light vector controls will be moved to another panels and fullscreen is a menu button. This allows control panels to use the full width of the program window
* The frame playback controls have been moved from the Sequences panel to its own separate area below the control panel area. This includes the current frame, speed, play/pause, restart and frame skip buttons
* Control panel height has been optimized to use as little space as possible to ensure controls are still visible on low resolution displays
* The Global Flags panel has been merged into the Model Data panel
* Added new panel: Cameras. This panel lists all available cameras for an asset and allows switching between them. Additionally, the field of view setting has been moved from Model Display to this panel, associated with the camera it belongs to
* Added new panel: Lighting. This panel lists all available lights in the scene for an asset and allows changing lighting parameters
    * Currently only the "Sky Light" light is available. This is the light that was used in previous versions of model viewer. The light vector controls that used CTRL+ left mouse dragging are now available as 2 angle dials for the X and Y axes, which control the light angles. These work much like Hammer's angle dials do: an X value of 270 (the initial value) points the light straight down. The Y value is the yaw angle

##### Model Display

* Added ground texture size setting. This setting determines the tile size of the ground texture, in units. This allows the texture to repeat instead of stretching to cover the ground. This option can be disabled and is disabled by default, which causes the texture to cover the entire ground

##### Sequences

* The panel layout has been greatly simplified due to many controls being moved to a separate area

##### Body Parts

* All bone controller settings have been added to the Body Parts panel and can be modified. Modifications are displayed in real-time for the current model. This includes the bone that the controller is attached to, the range it operates on, the controller index and which linear or rotational axis it affects
* Fixed a bug that caused the submodel combo box to always reset to the first value when changing the selected body
* Moved model info text to its own panel called Model Info
* Added model name edit box, mesh, vertex & normal count labels

##### Textures

* The UV map export has been reworked to allow the user to configure the image being generated for export:
    * Output format: When selecting a filename to save the UV map to the user can select a format as well. All image formats supported by Qt are available: https://doc.qt.io/qt-5/qimage.html#reading-and-writing-image-files
    * Image scale: the image scale can be configured as a percentage value of the original image to allow for greater resolution
    * UV Line Width: the width of the UV lines can be configured to make them more visible
    * Overlay on texture: the UV map can be overload on top of the original texture
    * Anti-alias lines: Anti-aliasing smooths the lines
    * Add alpha channel: An alpha channel is added. If the image format supports alpha channels it will be included
* The UV line width can be configured for normal UV map display as well
* It is now possible to import textures from and export textures to any supported image format. See here for a list of formats supported by Qt: https://doc.qt.io/qt-5/qimage.html#reading-and-writing-image-files

    Note that if a texture being imported is not an indexed 8 bit image it will be converted. Conversion may cause a loss of color depth.

    For best results users should manually convert images in image editing programs to avoid problems like this, but basic cases should work fine using built-in conversion.

    Additionally textures exported as any format that supports indexed 8 bit with a palette should retain the palette, with index-specific colors intact (e.g. transparent color for masked textures).
* Added texture filters options. Texture minifying and magnifying settings can be switched between point (pixel-perfect) and linear (interpolated). Mipmap settings can be switched between none (mipmapping disabled), point (pixel-perfect) and linear (interpolated). These options are initialized with the default settings configured in the options dialog and do not change when those settings change. Only newly loaded assets will use the new settings automatically to preserve asset-specific settings
* Added "Power Of 2 Textures" option. When changed all textures are re-uploaded with the new setting, which can affect texture blending and pixelation. This option is initialized with the default setting configured in the options dialog and does not change when that setting changes. Only newly loaded assets will use the new setting automatically to preserve asset-specific settings
* Added "Flat Shade" texture flag. This is set by the compiler for chrome textures but was not provided in model viewer. It can now be set separately
* Added "Mipmaps" texture flag. This enables mipmap generation. Mipmaps are only used when the mipmap filter is set to any option other than none
* Added edit box to change the name of a texture. Changing the name to match the pattern used for remapped textures will update the texture to contain remapped colors in real-time
* Set the texture view scale range to a minimum of 0.1 and a maximum of 20 with a single step value of 0.1. This makes viewing very large textures (> 512 width and/or height) possible without exceeding the viewport size

##### Bones

* Added a checkbox to toggle bone highlight drawing
* Made bone settings editable. Note: settings pertaining to position and rotation are in relative space, relative to the bone parent

##### Attachments

* Added a checkbox to toggle attachment highlight drawing
* Made attachment settings editable. Note: settings pertaining to position are in relative space, relative to the bone parent
* The attachment name is no longer red when empty. Instead the text `(empty)` is drawn in a light gray color

##### Hitboxes

* Added a checkbox to toggle hitbox highlight drawing
* Made hitbox settings editable. Note: settings pertaining to size  are in relative space, relative to the bone parent
