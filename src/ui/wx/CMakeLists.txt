target_sources(${TARGET_NAME}
	PRIVATE
		CwxOpenGL.cpp
		CwxOpenGL.h
		wxInclude.h
		wxLogging.cpp
		wxLogging.h)

add_subdirectory(shared)
add_subdirectory(utility)

