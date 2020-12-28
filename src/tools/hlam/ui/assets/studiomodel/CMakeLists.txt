target_sources(HLAM
	PRIVATE
		StudioModelAsset.cpp
		StudioModelAsset.hpp
		StudioModelColors.hpp
		StudioModelEditWidget.cpp
		StudioModelEditWidget.hpp
		StudioModelUndoCommands.cpp
		StudioModelUndoCommands.hpp
		StudioModelValidators.cpp
		StudioModelValidators.hpp)

add_subdirectory(compiler)
add_subdirectory(dockpanels)
