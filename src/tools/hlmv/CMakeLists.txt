#
#HLMV exe
#

set(TARGET_NAME HLMV)

find_package(OpenGL REQUIRED)

# Disable module based lookup (OpenAL Soft uses CONFIG mode and MODULE mode only works with the Creative Labs version)
find_package(OpenAL REQUIRED NO_MODULE)

if(NOT OPENGL_FOUND)
	message(FATAL_ERROR "Could not locate OpenGL library")
endif()

find_package(wxWidgets REQUIRED COMPONENTS gl adv propgrid core base)
include(${wxWidgets_USE_FILE})

add_executable(${TARGET_NAME})

check_winxp_support(${TARGET_NAME})

target_include_directories(${TARGET_NAME}
	PRIVATE
		${OPENGL_INCLUDE_DIR}
		${EXTERNAL_DIR}/AudioFile/include
		${SHARED_INCLUDEPATHS}
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/core
		${CMAKE_CURRENT_SOURCE_DIR}/engine
		${CMAKE_CURRENT_SOURCE_DIR}/stdlib)

target_compile_definitions(${TARGET_NAME}
	PRIVATE	
		${wxWidgets_DEFINITIONS}
		${SHARED_WX_DEFS})

target_sources(${TARGET_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:hlmv.rc>
		CHLMVState.cpp
		CHLMVState.h)

add_subdirectory(core)
add_subdirectory(cvar)
add_subdirectory(engine)
add_subdirectory(entity)
add_subdirectory(filesystem)
add_subdirectory(keyvalues)
add_subdirectory(settings)
add_subdirectory(soundsystem)
add_subdirectory(stdlib)
add_subdirectory(ui)

add_subdirectory(../shared ${CMAKE_CURRENT_BINARY_DIR}/tools/shared)
add_subdirectory(engine/shared ${CMAKE_CURRENT_BINARY_DIR}/engine/shared)
add_subdirectory(game ${CMAKE_CURRENT_BINARY_DIR}/game)
add_subdirectory(graphics ${CMAKE_CURRENT_BINARY_DIR}/graphics)
add_subdirectory(../../ui/wx ${CMAKE_CURRENT_BINARY_DIR}/ui/wx)

if(WIN32)
	find_library(GLEW glew32 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib)
else()
	find_library(GLEW libGLEW.so.2.0.0 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib)
endif()

target_link_libraries(${TARGET_NAME}
	PRIVATE
		${wxWidgets_LIBRARIES}
		${GLEW}
		${OPENGL_LIBRARIES}
		OpenAL::OpenAL
		${SHARED_DEPENDENCIES})

set_target_properties(${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${WX_SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_WX_LINK_FLAGS}")

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${SRC_DIR} FILES ${SOURCE_FILES})

if(WIN32)
	copy_dependencies(${TARGET_NAME} external/GLEW/lib glew32.dll)
else()
	copy_dependencies(${TARGET_NAME} external/GLEW/lib libGLEW.so.2.0.0)
endif()

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
