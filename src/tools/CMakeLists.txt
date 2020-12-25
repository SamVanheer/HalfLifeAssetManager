# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Process resources files
set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets Network REQUIRED)

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

add_library(EnTT INTERFACE)
add_library(EnTT::EnTT ALIAS EnTT)

target_include_directories(EnTT INTERFACE "../../external/EnTT/include")

add_subdirectory(hlam)
