target_sources(HLAM
	PRIVATE
		StudioModelAsset.cpp
		StudioModelAsset.hpp
		StudioModelColors.hpp
		StudioModelEditWidget.cpp
		StudioModelEditWidget.hpp
		StudioModelEditWidget.ui
		StudioModelUndoCommands.cpp
		StudioModelUndoCommands.hpp
		StudioModelValidators.cpp
		StudioModelValidators.hpp
		StudioModelView.cpp
		StudioModelView.hpp
		StudioModelView.ui)

add_subdirectory(compiler)
add_subdirectory(dockpanels)
