find_package(spdlog REQUIRED NO_DEFAULT_PATH  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/spdlog/lib/cmake/spdlog)

find_package(OpenGL REQUIRED)

# Disable module based lookup (OpenAL Soft uses CONFIG mode and MODULE mode only works with the Creative Labs version)
find_package(OpenAL REQUIRED NO_MODULE)

if(WIN32)
	find_library(GLEW glew32 PATHS ${EXTERNAL_DIR}/GLEW/lib)
else()
	find_library(GLEW libGLEW.so.2.0.0 PATHS ${EXTERNAL_DIR}/GLEW/lib)
endif()

add_library(Ogg STATIC IMPORTED)

set_target_properties(Ogg PROPERTIES IMPORTED_LOCATION ${EXTERNAL_DIR}/ogg/lib/libogg_static.lib)

target_include_directories(Ogg INTERFACE ${EXTERNAL_DIR}/ogg/include)

add_library(Vorbis STATIC IMPORTED)

set_target_properties(Vorbis PROPERTIES IMPORTED_LOCATION ${EXTERNAL_DIR}/vorbis/lib/libvorbis_static.lib)

target_include_directories(Vorbis INTERFACE ${EXTERNAL_DIR}/vorbis/include)

add_library(VorbisFile STATIC IMPORTED)

set_target_properties(VorbisFile PROPERTIES IMPORTED_LOCATION ${EXTERNAL_DIR}/vorbis/lib/libvorbisfile_static.lib)

# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE HLAM_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest tag name
execute_process(
    COMMAND git describe --abbrev=0 --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE HLAM_GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest commit hash
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE HLAM_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
		${CMAKE_CURRENT_BINARY_DIR}/version.hpp
		@ONLY)

if (MSVC)
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in
		${CMAKE_CURRENT_BINARY_DIR}/version_generated.rc
		@ONLY)
endif()

add_executable(HLAM WIN32)

target_include_directories(HLAM
	PRIVATE
		${EXTERNAL_DIR}/AudioFile/include
		${EXTERNAL_DIR}/GLEW/include
		${EXTERNAL_DIR}/GLM/include
		${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(HLAM
	PRIVATE	
		$<$<CXX_COMPILER_ID:MSVC>:
			UNICODE
			_UNICODE
			_CRT_SECURE_NO_WARNINGS
			_SCL_SECURE_NO_WARNINGS>
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
			FILE_OFFSET_BITS=64>
		IS_LITTLE_ENDIAN=${IS_LITTLE_ENDIAN_VALUE})

target_link_libraries(HLAM
	PRIVATE
		Qt5::Widgets
		Qt5::Network
		spdlog::spdlog_header_only 
		${GLEW}
		OpenGL::GL
		OpenAL::OpenAL
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:dl>
		Ogg
		Vorbis
		VorbisFile)

target_compile_options(HLAM
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/fp:strict>
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fPIC>)

target_link_options(HLAM
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/LTCG /INCREMENTAL:NO>
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wl,--exclude-libs,ALL>)

target_sources(HLAM
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:
			hlam.rc
			version.rc
		>
		Main.cpp
		resources.qrc)

add_subdirectory(application)
add_subdirectory(assets)
add_subdirectory(core)
add_subdirectory(engine)
add_subdirectory(engine/shared)
add_subdirectory(entity)
add_subdirectory(filesystem)
add_subdirectory(graphics)
add_subdirectory(qt)
add_subdirectory(soundsystem)
add_subdirectory(ui)
add_subdirectory(utility)

#Create filters
get_target_property(SOURCE_FILES HLAM SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# Add this after source_group to avoid errors with root paths
target_sources(HLAM
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/version.hpp)

if(WIN32)
	copy_dependencies(HLAM external/GLEW/lib glew32.dll)
else()
	copy_dependencies(HLAM external/GLEW/lib libGLEW.so.2.0.0)
endif()

add_custom_command(TARGET HLAM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:OpenAL::OpenAL>
        $<TARGET_FILE_DIR:HLAM>)

# Copy the Qt libraries over automatically
add_custom_command(TARGET HLAM POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Core>
        $<TARGET_FILE_DIR:HLAM>)

add_custom_command(TARGET HLAM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Gui>
        $<TARGET_FILE_DIR:HLAM>)

add_custom_command(TARGET HLAM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Widgets>
        $<TARGET_FILE_DIR:HLAM>)

add_custom_command(TARGET HLAM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Network>
        $<TARGET_FILE_DIR:HLAM>)

# Copy plugins so the program can run
copy_qt_dependencies(HLAM Qt5Core_PLUGINS)
copy_qt_dependencies(HLAM Qt5Gui_PLUGINS)
copy_qt_dependencies(HLAM Qt5Widgets_PLUGINS)
copy_qt_dependencies(HLAM Qt5Network_PLUGINS)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HLAM)

install(TARGETS HLAM
	RUNTIME DESTINATION bin)
