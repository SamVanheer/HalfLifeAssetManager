target_sources(HLMV
	PRIVATE
		EditorContext.cpp
		EditorContext.hpp
		HLMVMainWindow.cpp
		HLMVMainWindow.hpp
		HLMVMainWindow.ui
		SceneWidget.cpp
		SceneWidget.hpp)

add_subdirectory(assets)
add_subdirectory(camera_operators)
add_subdirectory(options)
