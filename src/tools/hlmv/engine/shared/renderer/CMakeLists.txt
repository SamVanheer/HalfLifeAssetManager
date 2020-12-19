target_sources(${TARGET_NAME}
	PRIVATE
		DrawConstants.hpp
		IRenderContext.hpp)

add_subdirectory(sprite)
add_subdirectory(studiomodel)
