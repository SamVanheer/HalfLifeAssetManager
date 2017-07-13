###################################################
#                                                 #
#                                                 #
#	Half-Life Tools CMake build file           	  #
#                                                 #
#                                                 #
###################################################
cmake_minimum_required( VERSION 3.6 )

include( cmake/InputFilesList.cmake )
include( cmake/MSVCRuntime.cmake )
include( cmake/PDBUtils.cmake )
include( cmake/Ternary.cmake )
include( cmake/WinXPSupport.cmake )

macro( copy_dependencies project_name base_dir )
	foreach( lib ${ARGN} )
		add_custom_command( TARGET ${project_name} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/${base_dir}/${lib}"
		"$<TARGET_FILE_DIR:${project_name}>"
	)
	endforeach()
endmacro()

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin )

project( HL_Tools )

if( UNIX )	
	set( LINUX_32BIT_FLAG "-m32" )
else()
	set( LINUX_32BIT_FLAG "" )
endif()

#C++14 support
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )

#Hide symbols from static libraries so they don't merge and cause issues
if( UNIX )
	if( CMAKE_COMPILER_IS_GNUCC )
		set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--exclude-libs,ALL" )
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--exclude-libs,ALL" )
	endif()
endif()

#Shared directories

#Save the shares sources into SHARED_SRCS
get_property(SHARED_SRCS GLOBAL PROPERTY SRCS_LIST)

clear_sources()

#Shared preprocessor definitions
#TODO: There's a file in CMake that can do this called TestBigEndian. See http://stackoverflow.com/questions/3877344/detect-endianness-with-cmake
#Consider using this
set( IS_LITTLE_ENDIAN_TARGET "1" CACHE BOOL "Whether the target system is little endian" )

#TODO really need to find a better way to do this
if( IS_LITTLE_ENDIAN_TARGET )
	set( IS_LITTLE_ENDIAN_VALUE "1" )
else()
	set( IS_LITTLE_ENDIAN_VALUE "0" )
endif()

if( WIN32 )
	set( SHARED_DEFS 
		UNICODE
		_UNICODE
		_CRT_SECURE_NO_WARNINGS
		_SCL_SECURE_NO_WARNINGS
	)
else()
endif()

set( SHARED_DEFS 
	${SHARED_DEFS}
	IS_LITTLE_ENDIAN=${IS_LITTLE_ENDIAN_VALUE}
)

if( WIN32 )
set( SHARED_WX_DEFS
	${SHARED_DEFS}
)
else()
set( SHARED_WX_DEFS
	_FILE_OFFSET_BITS=64
	WXUSINGDLL
	__WXGTK__
	${SHARED_DEFS}
)
endif()

#Shared include dirs
set( SHARED_INCLUDEPATHS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/stdlib"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/engine"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/core"
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/external/GLEW/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/external/GLM/include"
)

#Find shared dependencies

if( WIN32 )
	set( SHARED_DEPENDENCIES
	)
else()
	set( SHARED_DEPENDENCIES
		dl
		stdc++fs #C++17 experimental filesystem
	)
endif()

#TODO shared compile flags

if( WIN32 )
	set( SHARED_COMPILE_FLAGS
	)
else()
	set( SHARED_COMPILE_FLAGS
		"${LINUX_32BIT_FLAG} -fPIC"
	)
endif()

if( WIN32 )
	set( WX_SHARED_COMPILE_FLAGS 
		${SHARED_COMPILE_FLAGS}
	)
else()
	set( WX_SHARED_COMPILE_FLAGS
		${SHARED_COMPILE_FLAGS}
	)
endif()

#Shared linker settings
if( WIN32 )
	set( WX_LINKER_FLAGS "/SUBSYSTEM:WINDOWS" )
else()
	set( WX_LINKER_FLAGS "" )
endif()

set( SHARED_LINK_FLAGS
	"${LINUX_32BIT_FLAG}"
)

set( SHARED_WX_LINK_FLAGS
	"${SHARED_LINK_FLAGS} ${WX_LINKER_FLAGS}"
)

#Include source code
add_subdirectory( src )