#
#SoundSystem library
#

set( TARGET_NAME SoundSystem )

#Add in the shared sources
add_sources( ${SHARED_SRCS} )

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

add_library( ${TARGET_NAME} SHARED ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/external/FMOD/include
	${SHARED_INCLUDEPATHS}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE	
	${SHARED_DEFS}
)

if( WIN32 )
	find_library( FMOD fmod_vc.lib PATHS "${CMAKE_SOURCE_DIR}/external/FMOD/lib" )
else()
	find_library( FMOD fmod PATHS "${CMAKE_SOURCE_DIR}/external/FMOD/lib" )
endif()

target_link_libraries( ${TARGET_NAME}
	${FMOD}
	HLCore
	${SHARED_DEPENDENCIES}
)

set_target_properties( ${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()

if( WIN32 )
	copy_dependencies( ${TARGET_NAME} external/FMOD/lib fmod.dll )
else()
	copy_dependencies( ${TARGET_NAME} external/FMOD/lib libfmod.so )
endif()
