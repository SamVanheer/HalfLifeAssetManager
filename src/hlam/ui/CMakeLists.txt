target_sources(HLAM
	PRIVATE
		AboutDialog.cpp
		AboutDialog.hpp
		DockableWidget.hpp
		DragNDropEventFilter.hpp
		MainWindow.cpp
		MainWindow.hpp
		MainWindow.ui
		OpenGLGraphicsContext.hpp
		SceneWidget.cpp
		SceneWidget.hpp
		StateSnapshot.hpp)

add_subdirectory(camera_operators)
add_subdirectory(dialogs)
add_subdirectory(dockpanels)
add_subdirectory(options)
