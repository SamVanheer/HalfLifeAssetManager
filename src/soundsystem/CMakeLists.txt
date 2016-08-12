#
#SoundSystem library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add sources
add_sources(
	CSoundSystem.h
	CSoundSystem.cpp
	FMODDebug.h
	FMODDebug.cpp
)

add_subdirectory( shared )

add_subdirectory( ../lib ${CMAKE_CURRENT_BINARY_DIR}/lib )

preprocess_sources()

include_directories(
	"${CMAKE_SOURCE_DIR}/external/FMOD/include"
	"${SHARED_INCLUDEPATHS}"
)

add_library( SoundSystem SHARED ${PREP_SRCS} )

target_compile_definitions( SoundSystem PRIVATE	
	"${SHARED_DEFS}"
)

find_library( FMOD fmodex_vc.lib PATHS "${CMAKE_SOURCE_DIR}/external/FMOD/lib" )

target_link_libraries( SoundSystem
	"${FMOD}"
	HLCore
	"${SHARED_DEPENDENCIES}"
)

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()
