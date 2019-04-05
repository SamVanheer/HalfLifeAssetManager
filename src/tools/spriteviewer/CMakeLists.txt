#
#SpriteViewer exe
#

set( TARGET_NAME SpriteViewer )

#Add in the shared sources
add_sources( ${SHARED_SRCS} )

#Add sources
add_sources(
	CSpriteViewerState.h
	CSpriteViewerState.cpp
)

add_subdirectory( settings )
add_subdirectory( ui )

add_subdirectory( ../shared ${CMAKE_CURRENT_BINARY_DIR}/tools/shared )
add_subdirectory( ../../engine/shared ${CMAKE_CURRENT_BINARY_DIR}/engine/shared )
add_subdirectory( ../../lib ${CMAKE_CURRENT_BINARY_DIR}/lib )
add_subdirectory( ../../game ${CMAKE_CURRENT_BINARY_DIR}/game )
add_subdirectory( ../../ui/wx ${CMAKE_CURRENT_BINARY_DIR}/ui/wx )

preprocess_sources()

find_package( OpenGL REQUIRED )

if( NOT OPENGL_FOUND )
	MESSAGE( FATAL_ERROR "Could not locate OpenGL library" )
endif()

find_package( wxWidgets REQUIRED COMPONENTS gl adv propgrid core base )
include(${wxWidgets_USE_FILE})

add_executable( ${TARGET_NAME} ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${OPENGL_INCLUDE_DIR}
	${SHARED_INCLUDEPATHS}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE	
	${wxWidgets_DEFINITIONS}
	${SHARED_WX_DEFS}
)

if( WIN32 )
	find_library( GLEW glew32 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib )
else()
	find_library( GLEW libGLEW.so.2.0.0 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib )
endif()

target_link_libraries( ${TARGET_NAME}
	HLCore
	${wxWidgets_LIBRARIES}
	${GLEW}
	${OPENGL_LIBRARIES}
	${SHARED_DEPENDENCIES}
)

set_target_properties( ${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${WX_SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_WX_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()

if( WIN32 )
	copy_dependencies( ${TARGET_NAME} external/GLEW/lib glew32.dll )
endif()