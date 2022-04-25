find_package(OpenGL REQUIRED)

# Dependencies included as submodules.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(${EXTERNAL_DIR}/spdlog
	spdlog-binary-dir
	EXCLUDE_FROM_ALL)

option(BUILD_UTILS "" OFF)
add_subdirectory(${EXTERNAL_DIR}/glew
	glew-binary-dir
	EXCLUDE_FROM_ALL)

add_subdirectory(${EXTERNAL_DIR}/glm
	glm-binary-dir
	EXCLUDE_FROM_ALL)

set(CMAKE_POLICY_DEFAULT_CMP0071 NEW)
option(ALSOFT_EXAMPLES "" OFF)
option(ALSOFT_UTILS "" OFF)
add_subdirectory(${EXTERNAL_DIR}/OpenAL
	OpenAL-binary-dir
	EXCLUDE_FROM_ALL)

unset(CMAKE_POLICY_DEFAULT_CMP0071)

set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

add_subdirectory(${EXTERNAL_DIR}/ogg
	ogg-binary-dir
	EXCLUDE_FROM_ALL)

add_subdirectory(${EXTERNAL_DIR}/vorbis
	vorbis-binary-dir
	EXCLUDE_FROM_ALL)

unset(CMAKE_POLICY_DEFAULT_CMP0048)

add_subdirectory(${EXTERNAL_DIR}/AudioFile
	AudioFile-binary-dir
	EXCLUDE_FROM_ALL)
	
set_target_properties(
	libglew_shared
	build_version
	common
	OpenAL
	ogg
	vorbis
	vorbisfile
	PROPERTIES
		FOLDER "Dependencies")

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

target_compile_definitions(HLAM
	PRIVATE
		QT_MESSAGELOGCONTEXT
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
		libglew_shared
		OpenGL::GL
		OpenAL::OpenAL
		glm::glm
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:dl>
		Ogg::ogg
		Vorbis::vorbis
		Vorbis::vorbisfile
		AudioFile)

target_compile_options(HLAM
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/MP /fp:strict>
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fPIC>)

target_link_options(HLAM
	PRIVATE
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

add_custom_command(TARGET HLAM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:libglew_shared>
        $<TARGET_FILE_DIR:HLAM>)

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
