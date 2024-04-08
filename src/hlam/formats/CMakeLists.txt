target_sources(HLAM
	PRIVATE
		activity.hpp
		DrawConstants.hpp
		liblist/LiblistReader.cpp
		liblist/LiblistReader.hpp)

add_subdirectory(studiomodel)
