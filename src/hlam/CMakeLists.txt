if (MSVC)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version_generated.rc @ONLY)
endif()

add_executable(HLAM WIN32)

add_dependencies(HLAM ProjectInfo)

target_compile_features(HLAM
	PRIVATE
		cxx_std_20)

target_include_directories(HLAM
	PRIVATE
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
		$<$<CXX_COMPILER_ID:MSVC>:hlam.rc version.rc>
		Main.cpp
		resources.qrc)

add_subdirectory(application)
add_subdirectory(assets)
add_subdirectory(entity)
add_subdirectory(filesystem)
add_subdirectory(formats)
add_subdirectory(graphics)
add_subdirectory(plugins)
add_subdirectory(qt)
add_subdirectory(settings)
add_subdirectory(soundsystem)
add_subdirectory(ui)
add_subdirectory(utility)

#Create filters
get_target_property(SOURCE_FILES HLAM SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

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

install(TARGETS HLAM
	RUNTIME DESTINATION bin)
