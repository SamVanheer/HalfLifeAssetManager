#
#Renderer library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

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

include_directories(
	${OPENGL_INCLUDE_DIR}
	"${SHARED_INCLUDEPATHS}"
)

add_library( Renderer SHARED ${PREP_SRCS} )

target_compile_definitions( Renderer PRIVATE
	"${SHARED_DEFS}"
)

target_link_libraries( Renderer
	HLStdLib
	HLCore
	${OPENGL_LIBRARIES}
	${SHARED_DEPENDENCIES}
)

set_target_properties( Renderer 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()
