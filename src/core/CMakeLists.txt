#
#HLCore library
#

set( TARGET_NAME HLCore )

#Add in the shared sources
add_sources( ${SHARED_SRCS} )

#Add sources
add_sources(
	LibHLCore.h
	LibHLCore.cpp
)

add_subdirectory( shared )

preprocess_sources()

add_library( ${TARGET_NAME} SHARED ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${SHARED_INCLUDEPATHS}
)

#TODO: CMake adds exports macros automatically - Solokiller
target_compile_definitions( ${TARGET_NAME} PRIVATE	
	${SHARED_DEFS}
	LIBHLCORE_EXPORTS
)

target_link_libraries( ${TARGET_NAME}
	Keyvalues
	HLStdLib
	${SHARED_DEPENDENCIES}
)

set_target_properties( ${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()