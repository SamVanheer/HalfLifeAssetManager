target_sources(${TARGET_NAME}
	PRIVATE
		DrawConstants.h
		IRenderContext.h
		IRendererLibrary.h)

add_subdirectory(sprite)
add_subdirectory(studiomodel)
