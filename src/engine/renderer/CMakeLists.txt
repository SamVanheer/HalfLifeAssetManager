#
#Renderer library
#

set( TARGET_NAME Renderer )

#Add in the shared sources
add_sources( ${SHARED_SRCS} )

#Add sources

add_sources(
	CBaseRenderContext.h
	CBaseRenderContext.cpp
	CRendererLibrary.h
	CRendererLibrary.cpp
)

add_subdirectory( gl )
add_subdirectory( sprite )
add_subdirectory( studiomodel )
add_subdirectory( util )

add_subdirectory( ../shared ${CMAKE_CURRENT_BINARY_DIR}/shared )

add_subdirectory( ../../lib ${CMAKE_CURRENT_BINARY_DIR}/lib )

preprocess_sources()

find_package( OpenGL REQUIRED )

if( NOT OPENGL_FOUND )
	MESSAGE( FATAL_ERROR "Could not locate OpenGL library" )
endif()

add_library( ${TARGET_NAME} SHARED ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${OPENGL_INCLUDE_DIR}
	${SHARED_INCLUDEPATHS}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE
	${SHARED_DEFS}
)

target_link_libraries( ${TARGET_NAME}
	HLStdLib
	HLCore
	${OPENGL_LIBRARIES}
	${SHARED_DEPENDENCIES}
)

set_target_properties( ${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()
