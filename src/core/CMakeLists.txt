#
#HLCore library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add sources
add_sources(
	LibHLCore.h
	LibHLCore.cpp
)

add_subdirectory( shared )

preprocess_sources()

include_directories(
	"${SHARED_INCLUDEPATHS}"
)

add_library( HLCore SHARED ${PREP_SRCS} )

target_compile_definitions( HLCore PRIVATE	
	"${SHARED_DEFS}"
	LIBHLCORE_EXPORTS
)

target_link_libraries( HLCore
	Keyvalues
	HLStdLib
	"${SHARED_DEPENDENCIES}"
)

set_target_properties( HLCore 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()