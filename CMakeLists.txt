cmake_minimum_required(VERSION 3.18)

include(cmake/WinXPSupport.cmake)

macro(copy_dependencies project_name base_dir)
	foreach(lib ${ARGN})
		add_custom_command(TARGET ${project_name} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_SOURCE_DIR}/${base_dir}/${lib}"
			"$<TARGET_FILE_DIR:${project_name}>")
	endforeach()
endmacro()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin)

project(HL_Tools)

set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN_TARGET)

if(IS_BIG_ENDIAN_TARGET)
	set(IS_LITTLE_ENDIAN_VALUE "0")
else()
	set(IS_LITTLE_ENDIAN_VALUE "1")
endif()

add_subdirectory(src)
