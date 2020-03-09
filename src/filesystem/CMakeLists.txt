#
#FileSystem library
#

set(TARGET_NAME FileSystem)

add_library(${TARGET_NAME} SHARED)

check_winxp_support(${TARGET_NAME})

target_include_directories(${TARGET_NAME}
	PRIVATE
		${SHARED_INCLUDEPATHS})

target_compile_definitions(${TARGET_NAME}
	PRIVATE	
		${SHARED_DEFS})

target_sources(${TARGET_NAME}
	PRIVATE
		CFileSystem.cpp
		CFileSystem.h
		FileSystemConstants.cpp
		FileSystemConstants.h
		IFileSystem.h)

add_subdirectory(../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

set_target_properties(${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}")

target_link_libraries(${TARGET_NAME}
	PRIVATE
		HLStdLib)

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${SRC_DIR} FILES ${SOURCE_FILES})
