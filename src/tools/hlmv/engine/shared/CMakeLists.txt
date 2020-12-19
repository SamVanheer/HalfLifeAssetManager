target_sources(${TARGET_NAME}
	PRIVATE
		activity.cpp
		activity.hpp
		activitymap.hpp)

add_subdirectory(renderer)
add_subdirectory(sprite)
add_subdirectory(studiomodel)
