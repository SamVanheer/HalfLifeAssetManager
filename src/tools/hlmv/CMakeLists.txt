#
#HLMV exe
#

set( TARGET_NAME HLMV )

#Add in the shared sources
add_sources( ${SHARED_SRCS} )

#Add sources
add_sources(
	CHLMVState.h
	CHLMVState.cpp
)

add_subdirectory( entity )
add_subdirectory( settings )
add_subdirectory( ui )

add_subdirectory( ../shared ${CMAKE_CURRENT_BINARY_DIR}/tools/shared )
add_subdirectory( ../../engine/shared ${CMAKE_CURRENT_BINARY_DIR}/engine/shared )
add_subdirectory( ../../lib ${CMAKE_CURRENT_BINARY_DIR}/lib )
add_subdirectory( ../../game ${CMAKE_CURRENT_BINARY_DIR}/game )
add_subdirectory( ../../graphics ${CMAKE_CURRENT_BINARY_DIR}/graphics )
add_subdirectory( ../../ui/wx ${CMAKE_CURRENT_BINARY_DIR}/ui/wx )

preprocess_sources()

find_package( OpenGL REQUIRED )

if( NOT OPENGL_FOUND )
	MESSAGE( FATAL_ERROR "Could not locate OpenGL library" )
endif()

if( WIN32 )
	set( WXWIDGETS_LIB_INCLUDE ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll/mswud )
else()
	set( WXWIDGETS_LIB_INCLUDE ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/Linux_gtk/wx/include/gtk2-unicode-3.1 )
endif()

add_executable( ${TARGET_NAME} ${PREP_SRCS} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${WXWIDGETS_LIB_INCLUDE}
	${CMAKE_SOURCE_DIR}/external/wxWidgets/include
	${OPENGL_INCLUDE_DIR}
	${SHARED_INCLUDEPATHS}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE	
	WXUSINGDLL
	${SHARED_WX_DEFS}
)

if( WIN32 )
	find_library( GLEW glew32 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib )
else()
	find_library( GLEW libGLEW.so.2.0.0 PATHS ${CMAKE_SOURCE_DIR}/external/GLEW/lib )
endif()

if( WIN32 )
	find_library( WXBASED wxbase31ud PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	find_library( WXCORED wxmsw31ud_core PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	find_library( WXGLD wxmsw31ud_gl PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	find_library( WXADVD wxmsw31ud_adv PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	find_library( WXPROPGRIDD wxmsw31ud_propgrid PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )

	find_library( WXBASE wxbase31u PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	find_library( WXCORE wxmsw31u_core PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	find_library( WXGL wxmsw31u_gl PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	find_library( WXADV wxmsw31u_adv PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	find_library( WXPROPGRID wxmsw31u_propgrid PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc140_dll )
	
	set( WXWIDGETS_LIBS
		${WXPROPGRID} 	optimized
		${WXADV} 		optimized
		${WXGL} 		optimized
		${WXCORE} 		optimized
		${WXBASE} 		optimized
		${WXPROPGRIDD} 	#debug
		${WXADVD} 		#debug
		${WXGLD} 		#debug
		${WXCORED} 		#debug
		${WXBASED} 		#debug
	)
else()
	find_library( WXBASE wx_baseu-3.1 PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/Linux_gtk )
	find_library( WXCORE wx_gtk2u_core-3.1 PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/Linux_gtk )
	find_library( WXGL wx_gtk2u_gl-3.1 PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/Linux_gtk )
	find_library( WXADV wx_gtk2u_adv-3.1 PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/Linux_gtk )
	find_library( WXPROPGRID wx_gtk2u_propgrid-3.1 PATHS ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/Linux_gtk )
	
	set( WXWIDGETS_LIBS
		${WXPROPGRID}
		${WXADV}
		${WXGL}
		${WXCORE}
		${WXBASE}
	)
endif()

target_link_libraries( ${TARGET_NAME}
	HLCore
	Keyvalues
	${WXWIDGETS_LIBS}
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
	#Copy wxWidgets libraries
	copy_dependencies( ${TARGET_NAME} external/wxWidgets/lib/vc140_dll
		wxbase310ud_vc140.dll
		wxmsw310ud_core_vc140.dll
		wxmsw310ud_gl_vc140.dll
		wxmsw310ud_adv_vc140.dll
		wxmsw310ud_propgrid_vc140.dll
		wxbase310u_vc140.dll
		wxmsw310u_core_vc140.dll
		wxmsw310u_gl_vc140.dll
		wxmsw310u_adv_vc140.dll
		wxmsw310u_propgrid_vc140.dll
	)
	
	copy_dependencies( ${TARGET_NAME} external/GLEW/lib glew32.dll )
else()
	copy_dependencies( ${TARGET_NAME} external/GLEW/lib libGLEW.so.2.0.0 )
endif()

set_property( DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME} )
