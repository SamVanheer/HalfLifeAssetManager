#
#HLStdLib library
#

set(TARGET_NAME HLStdLib)

add_library(${TARGET_NAME} STATIC)

check_winxp_support(${TARGET_NAME})

target_include_directories(${TARGET_NAME}
	PRIVATE
		${SHARED_INCLUDEPATHS}
		${SRC_DIR}/tools/hlmv/engine)

target_compile_definitions(${TARGET_NAME}
	PRIVATE	
		${SHARED_DEFS})

add_subdirectory(app)
add_subdirectory(cvar)
add_subdirectory(graphics)
add_subdirectory(lib)
add_subdirectory(settings)
add_subdirectory(utility)

set_target_properties(${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}")

if(WIN32)
	find_library(GLEW glew32 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib)
else()
	find_library(GLEW libGLEW.so.2.0.0 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib)
endif()

find_package(OpenGL REQUIRED)

if(NOT OPENGL_FOUND)
	message(FATAL_ERROR "Could not locate OpenGL library")
endif()

target_link_libraries(${TARGET_NAME}
	PUBLIC
	Keyvalues
		${GLEW}
		${OPENGL_LIBRARIES}
		${SHARED_DEPENDENCIES})

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${SRC_DIR} FILES ${SOURCE_FILES})
