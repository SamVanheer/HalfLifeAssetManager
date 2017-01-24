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

if( WIN32 )
	find_library( FMOD fmod_vc.lib PATHS "${CMAKE_SOURCE_DIR}/external/FMOD/lib" )
else()
	find_library( FMOD fmod PATHS "${CMAKE_SOURCE_DIR}/external/FMOD/lib" )
endif()

target_link_libraries( SoundSystem
	"${FMOD}"
	HLCore
	"${SHARED_DEPENDENCIES}"
)

set_target_properties( SoundSystem 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()

if( WIN32 )
	copy_dependencies( SoundSystem external/FMOD/lib fmod.dll )
else()
	copy_dependencies( SoundSystem external/FMOD/lib libfmod.so )
endif()
