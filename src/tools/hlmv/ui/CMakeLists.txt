target_sources(HLMV
	PRIVATE
		EditorContext.cpp
		EditorContext.hpp
		FullscreenWidget.cpp
		FullscreenWidget.hpp
		HLMVMainWindow.cpp
		HLMVMainWindow.hpp
		HLMVMainWindow.ui
		IInputSink.hpp
		SceneWidget.cpp
		SceneWidget.hpp)

add_subdirectory(assets)
add_subdirectory(camera_operators)
add_subdirectory(options)
