#
#CVar library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add sources
add_sources(
	CCVarSystem.h
	CCVarSystem.cpp
)

add_subdirectory( ../lib ${CMAKE_CURRENT_BINARY_DIR}/lib )

preprocess_sources()

include_directories(
	"${CMAKE_SOURCE_DIR}/external/FMOD/include"
	"${SHARED_INCLUDEPATHS}"
)

add_library( CVar SHARED ${PREP_SRCS} )

target_compile_definitions( CVar PRIVATE	
	"${SHARED_DEFS}"
)

target_link_libraries( CVar
	HLStdLib
	HLCore
	"${SHARED_DEPENDENCIES}"
)

set_target_properties( CVar 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()
