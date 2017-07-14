#
#FileSystem library
#

set( TARGET_NAME FileSystem )

#Add in the shared sources
add_sources( ${SHARED_SRCS} )

#Add sources

add_sources(
	CFileSystem.h
	CFileSystem.cpp
	FileSystemConstants.h
	FileSystemConstants.cpp
	IFileSystem.h
)

add_subdirectory( ../lib ${CMAKE_CURRENT_BINARY_DIR}/lib )

preprocess_sources()

add_library( ${TARGET_NAME} SHARED ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${SHARED_INCLUDEPATHS}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE	
	${SHARED_DEFS}
)

set_target_properties( ${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

target_link_libraries( ${TARGET_NAME}
	HLStdLib
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()
