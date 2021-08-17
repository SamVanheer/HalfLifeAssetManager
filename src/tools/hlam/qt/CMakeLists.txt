target_sources(HLAM
	PRIVATE
		ByteLengthValidator.hpp
		HashFunctions.hpp
		QtLogging.cpp
		QtLogging.hpp
		QtLogSink.hpp
		QtUtilities.cpp
		QtUtilities.hpp)

add_subdirectory(widgets)
