target_sources(${TARGET_NAME}
	PRIVATE
		activity.cpp
		activity.h
		activitymap.h)

add_subdirectory(renderer)
add_subdirectory(sprite)
add_subdirectory(studiomodel)
