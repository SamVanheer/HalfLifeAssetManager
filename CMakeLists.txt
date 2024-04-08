cmake_minimum_required(VERSION 3.25)

set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_SOURCE_DIR}/vcpkg_config/triplets)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR})

set(HLAM_VERSION_MAJOR 3)
set(HLAM_VERSION_MINOR 0)
set(HLAM_VERSION_PATCH 0)

project(HalfLifeAssetManager
	VERSION ${HLAM_VERSION_MAJOR}.${HLAM_VERSION_MINOR}.${HLAM_VERSION_PATCH}
	DESCRIPTION "Half-Life Asset Manager"
	LANGUAGES CXX)

# Default to Alpha. CI builds will always be Alpha builds.
set(BUILD_TYPE "Alpha" "Beta" "Public")
set(HLAM_BUILD_TYPE "Alpha" CACHE STRING "What type of build you're making")
set_property(CACHE HLAM_BUILD_TYPE PROPERTY STRINGS ${BUILD_TYPE})

list(FIND BUILD_TYPE "${HLAM_BUILD_TYPE}" BUILD_TYPE_INDEX)
if(BUILD_TYPE_INDEX EQUAL -1)
	message(FATAL_ERROR "HLAM_BUILD_TYPE must be one of: ${BUILD_TYPE}")
endif()

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
		-D HLAM_BUILD_TYPE=${HLAM_BUILD_TYPE}
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

# TODO: need to move everything from src/hlam to src once WIP stuff is done

if (MSVC)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/hlam/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version_generated.rc @ONLY)
endif()

add_executable(HLAM WIN32)

add_dependencies(HLAM ProjectInfo)

target_compile_features(HLAM
	PRIVATE
		cxx_std_20)

target_include_directories(HLAM
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src/hlam
		${CMAKE_BINARY_DIR})

target_compile_definitions(HLAM
	PRIVATE
		QT_MESSAGELOGCONTEXT
		$<$<CXX_COMPILER_ID:MSVC>:
			UNICODE
			_UNICODE
			_CRT_SECURE_NO_WARNINGS
			_SCL_SECURE_NO_WARNINGS>
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
			FILE_OFFSET_BITS=64>)

target_link_libraries(HLAM
	PRIVATE
		Qt5::Widgets
		Qt5::Network
		fmt::fmt
		spdlog::spdlog_header_only 
		OpenAL::OpenAL
		glm::glm
		${CMAKE_DL_LIBS}
		libnyquist)

target_compile_options(HLAM
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/MP /fp:strict>
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fPIC>)

target_link_options(HLAM
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wl,--exclude-libs,ALL>)

target_sources(HLAM
	PRIVATE
		src/hlam/Main.cpp
		src/hlam/resources.qrc)

if (MSVC)
	target_sources(HLAM PRIVATE src/hlam/hlam.rc src/hlam/version.rc)
endif()

add_subdirectory(src/hlam/application)
add_subdirectory(src/hlam/entity)
add_subdirectory(src/hlam/filesystem)
add_subdirectory(src/hlam/formats)
add_subdirectory(src/hlam/graphics)
add_subdirectory(src/hlam/plugins)
add_subdirectory(src/hlam/qt)
add_subdirectory(src/hlam/settings)
add_subdirectory(src/hlam/soundsystem)
add_subdirectory(src/hlam/ui)
add_subdirectory(src/hlam/utility)

#Create filters
get_target_property(SOURCE_FILES HLAM SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/hlam FILES ${SOURCE_FILES})

# Add this after source_group to avoid errors with root paths
target_sources(HLAM PRIVATE ${CMAKE_BINARY_DIR}/ProjectInfo.hpp)

# Copy runtime dependencies automatically for dev builds.
add_custom_command(TARGET HLAM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:OpenAL::OpenAL> $<TARGET_FILE_DIR:HLAM>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:HLAM>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:HLAM>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:HLAM>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Network> $<TARGET_FILE_DIR:HLAM>
)

# Copy plugins so the program can run
copy_qt_dependencies(HLAM Qt5Core_PLUGINS)
copy_qt_dependencies(HLAM Qt5Gui_PLUGINS)
copy_qt_dependencies(HLAM Qt5Widgets_PLUGINS)
copy_qt_dependencies(HLAM Qt5Network_PLUGINS)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HLAM)

# On Windows the executable is installed in its own directory.
# On Linux it's installed in a global location and must follow platform conventions.
if (WIN32)
	install(TARGETS HLAM RUNTIME DESTINATION .)
	install(FILES "docs/manual/HalfLifeAssetManagerManual.pdf" DESTINATION "manual")
else()
	include(GNUInstallDirs)

	# Qt searches in "dataroot/orgname/appname" which is why this is nested.
	set(APP_LOCAL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/Half-Life Asset Manager/Half-Life Asset Manager")

	install(TARGETS HLAM RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES "distribution/linux/hlam.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
	install(FILES "assets/hlam_icon2_golden_finished.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps" RENAME "hlam.png")
	install(FILES "docs/manual/HalfLifeAssetManagerManual.pdf" DESTINATION "${APP_LOCAL_DIR}")
	
	# CPack is only used for Linux versions.
	set(CPACK_PACKAGE_CONTACT "https://github.com/SamVanheer/HalfLifeAssetManager")
	set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
	set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
	set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

	include(CPack)
endif()
