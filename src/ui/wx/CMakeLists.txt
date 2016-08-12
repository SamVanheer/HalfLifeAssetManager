add_sources(
	CwxOpenGL.h
	CwxOpenGL.cpp
	wxInclude.h
	wxLogging.h
	wxLogging.cpp
)

add_includes(
	CwxOpenGL.h
	wxInclude.h
	wxLogging.h
)

add_subdirectory( shared )
add_subdirectory( utility )

