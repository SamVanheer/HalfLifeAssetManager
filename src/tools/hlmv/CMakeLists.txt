set(TARGET_NAME HLMV)

find_package(OpenGL REQUIRED)

# Disable module based lookup (OpenAL Soft uses CONFIG mode and MODULE mode only works with the Creative Labs version)
find_package(OpenAL REQUIRED NO_MODULE)

if(WIN32)
	find_library(GLEW glew32 PATHS ${EXTERNAL_DIR}/GLEW/lib)
else()
	find_library(GLEW libGLEW.so.2.0.0 PATHS ${EXTERNAL_DIR}/GLEW/lib)
endif()

find_package(wxWidgets REQUIRED COMPONENTS gl adv propgrid core base)
include(${wxWidgets_USE_FILE})

add_library(Ogg STATIC IMPORTED)

set_target_properties(Ogg PROPERTIES IMPORTED_LOCATION ${EXTERNAL_DIR}/ogg/lib/libogg_static.lib)

target_include_directories(Ogg INTERFACE ${EXTERNAL_DIR}/ogg/include)

add_library(Vorbis STATIC IMPORTED)

set_target_properties(Vorbis PROPERTIES IMPORTED_LOCATION ${EXTERNAL_DIR}/vorbis/lib/libvorbis_static.lib)

target_include_directories(Vorbis INTERFACE ${EXTERNAL_DIR}/vorbis/include)

add_library(VorbisFile STATIC IMPORTED)

set_target_properties(VorbisFile PROPERTIES IMPORTED_LOCATION ${EXTERNAL_DIR}/vorbis/lib/libvorbisfile_static.lib)

add_executable(${TARGET_NAME})

check_winxp_support(${TARGET_NAME})

target_include_directories(${TARGET_NAME}
	PRIVATE
		${EXTERNAL_DIR}/AudioFile/include
		${EXTERNAL_DIR}/GLEW/include
		${EXTERNAL_DIR}/GLM/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/core
		${CMAKE_CURRENT_SOURCE_DIR}/engine
		${CMAKE_CURRENT_SOURCE_DIR}/stdlib)

target_compile_definitions(${TARGET_NAME}
	PRIVATE	
		${wxWidgets_DEFINITIONS}
		$<$<CXX_COMPILER_ID:MSVC>:
			UNICODE
			_UNICODE
			_CRT_SECURE_NO_WARNINGS
			_SCL_SECURE_NO_WARNINGS>
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
			FILE_OFFSET_BITS=64
			WXUSINGDLL
			__WXGTK__>
		IS_LITTLE_ENDIAN=${IS_LITTLE_ENDIAN_VALUE})

target_link_libraries(${TARGET_NAME}
	PRIVATE
		${wxWidgets_LIBRARIES}
		${GLEW}
		OpenGL::GL
		OpenAL::OpenAL
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:dl>
		Ogg
		Vorbis
		VorbisFile)

target_compile_options(${TARGET_NAME}
	PRIVATE
		${WX_SHARED_COMPILE_FLAGS}
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-m32 -fPIC>)

target_link_options(${TARGET_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:WINDOWS>
		$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-m32 --exclude-libs,ALL>)

target_sources(${TARGET_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:hlmv.rc>
		CHLMVState.cpp
		CHLMVState.h
		Credits.cpp
		Credits.h)

add_subdirectory(core)
add_subdirectory(cvar)
add_subdirectory(engine)
add_subdirectory(engine/shared)
add_subdirectory(entity)
add_subdirectory(filesystem)
add_subdirectory(game)
add_subdirectory(graphics)
add_subdirectory(keyvalues)
add_subdirectory(settings)
add_subdirectory(soundsystem)
add_subdirectory(ui)
add_subdirectory(utility)
add_subdirectory(wx)

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

if(WIN32)
	copy_dependencies(${TARGET_NAME} external/GLEW/lib glew32.dll)
else()
	copy_dependencies(${TARGET_NAME} external/GLEW/lib libGLEW.so.2.0.0)
endif()

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
