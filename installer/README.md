To create the installer you will need to put the program executable and all required Qt libraries in the bin directory, and place the Visual Studio x86 redistributable in the redist directory.
The PDF manual generated from [HalfLifeAssetManagerManual.tex](/docs/manual/HalfLifeAssetManagerManual.tex) should be placed in the manual directory.

Also required are the following libraries included with Qt's installation, needed for ANGLE-based OpenGL rendering:
* libEGL.dll
* libGLESv2.dll
* d3dcompiler_XX.dll (XX is the version number of the compiler that ANGLE is compiled against)
* opengl32sw.dll

See this for more information on packaging Qt applications: https://doc.qt.io/qt-5/windows-deployment.html
