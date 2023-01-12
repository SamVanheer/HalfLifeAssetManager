cmake_minimum_required(VERSION 3.25)

set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_SOURCE_DIR}/vcpkg_config/triplets)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin)

set(HLAM_VERSION_MAJOR 2)
set(HLAM_VERSION_MINOR 0)
set(HLAM_VERSION_PATCH 0)

project(HalfLifeAssetManager
	VERSION ${HLAM_VERSION_MAJOR}.${HLAM_VERSION_MINOR}.${HLAM_VERSION_PATCH}
	DESCRIPTION "Half-Life Asset Manager"
	LANGUAGES CXX)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Process resources files
set(CMAKE_AUTORCC ON)

find_package(Git REQUIRED)

add_custom_target(ProjectInfo
	COMMAND ${CMAKE_COMMAND}
		-D HLAM_VERSION_MAJOR=${HLAM_VERSION_MAJOR}
		-D HLAM_VERSION_MINOR=${HLAM_VERSION_MINOR}
		-D HLAM_VERSION_PATCH=${HLAM_VERSION_PATCH}
		-D GIT_EXECUTABLE=${GIT_EXECUTABLE}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/UpdateProjectInfo.cmake
	BYPRODUCTS ${CMAKE_BINARY_DIR}/ProjectInfo.hpp)

# Find the QtWidgets library
find_package(Qt5 CONFIG COMPONENTS Widgets Network REQUIRED)

function(copy_qt_dependencies target_name plugins)
	foreach(plugin ${${plugins}})
		# Copy plugins to the directory relative to the plugins directory in the Qt install directory
		get_target_property(TARGET_LOCATION ${plugin} LOCATION)
		get_filename_component(PARENT_DIR ${TARGET_LOCATION} DIRECTORY)
		get_filename_component(TARGET_DIR_NAME ${PARENT_DIR} NAME)
		
		add_custom_command(TARGET ${target_name} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${target_name}>/${TARGET_DIR_NAME})
	  
		add_custom_command(TARGET ${target_name} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<TARGET_FILE:${plugin}>
				$<TARGET_FILE_DIR:${target_name}>/${TARGET_DIR_NAME})
	endforeach()
endfunction()

find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(libnyquist CONFIG REQUIRED)

add_subdirectory(src)
