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

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()
