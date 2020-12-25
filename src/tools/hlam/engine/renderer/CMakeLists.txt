target_sources(HLAM
	PRIVATE
		CBaseRenderContext.cpp
		CBaseRenderContext.hpp)

add_subdirectory(gl)
add_subdirectory(sprite)
add_subdirectory(studiomodel)
add_subdirectory(util)
