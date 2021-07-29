rem If this fails, make sure the path to WinRAR's Rar.exe is in your PATH environment variable

set RAR="Rar.exe"
set ARCHIVE_FILENAME="HalfLifeAssetManagerPortable.rar"

del %ARCHIVE_FILENAME%

%RAR% a -ep1 %ARCHIVE_FILENAME% "packages/SamVanheer.HalfLifeAssetManager/data"

%RAR% rn %ARCHIVE_FILENAME% "data" "Half-Life Asset Manager"
