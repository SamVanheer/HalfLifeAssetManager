#
#HLStdLib library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add sources

add_subdirectory( app )
add_subdirectory( cvar )
add_subdirectory( graphics )
add_subdirectory( html )
add_subdirectory( lib )
add_subdirectory( settings )
add_subdirectory( utility )

preprocess_sources()

include_directories(
	"${SHARED_INCLUDEPATHS}"
)

add_library( HLStdLib STATIC ${PREP_SRCS} )

target_compile_definitions( HLStdLib PRIVATE	
	"${SHARED_DEFS}"
)

set_target_properties( HLStdLib 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

if( WIN32 )
	find_library( GLEW glew32 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib )
else()
	find_library( GLEW libGLEW.so.2.0.0 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib )
endif()

find_package( OpenGL REQUIRED )

if( NOT OPENGL_FOUND )
	MESSAGE( FATAL_ERROR "Could not locate OpenGL library" )
endif()

target_link_libraries( HLStdLib
	Keyvalues
	${GLEW}
	${OPENGL_LIBRARIES}
	${SHARED_DEPENDENCIES}
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()
