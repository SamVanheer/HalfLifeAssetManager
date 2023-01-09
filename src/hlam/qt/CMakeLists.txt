target_sources(HLAM
	PRIVATE
		ByteLengthValidator.hpp
		HashFunctions.hpp
		QtLogging.hpp
		QtLogSink.hpp
		QtUtilities.cpp
		QtUtilities.hpp)

add_subdirectory(widgets)
