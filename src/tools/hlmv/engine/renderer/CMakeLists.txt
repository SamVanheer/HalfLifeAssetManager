target_sources(${TARGET_NAME}
	PRIVATE
		CBaseRenderContext.cpp
		CBaseRenderContext.h)

add_subdirectory(gl)
add_subdirectory(sprite)
add_subdirectory(studiomodel)
add_subdirectory(util)
