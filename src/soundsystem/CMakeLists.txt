#
#SoundSystem library
#

set( TARGET_NAME SoundSystem )

# Disable module based lookup (OpenAL Soft uses CONFIG mode and MODULE mode only works with the Creative Labs version)
find_package(OpenAL REQUIRED NO_MODULE)

#Add in the shared sources
add_sources( ${SHARED_SRCS} )

#Add sources
add_sources(
	CSoundSystem.h
	CSoundSystem.cpp
)

add_subdirectory( shared )

add_subdirectory( ../lib ${CMAKE_CURRENT_BINARY_DIR}/lib )

preprocess_sources()

add_library( ${TARGET_NAME} SHARED ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${EXTERNAL_DIR}/AudioFile/include
	${SHARED_INCLUDEPATHS}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE	
	${SHARED_DEFS}
)

target_link_libraries( ${TARGET_NAME}
	HLCore
	${SHARED_DEPENDENCIES}
	OpenAL::OpenAL
)

set_target_properties( ${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()
