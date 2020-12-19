target_sources(${TARGET_NAME}
	PRIVATE
		CBaseRenderContext.cpp
		CBaseRenderContext.hpp)

add_subdirectory(gl)
add_subdirectory(sprite)
add_subdirectory(studiomodel)
add_subdirectory(util)
