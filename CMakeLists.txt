###################################################
#                                                 #
#                                                 #
#	Half-Life Tools CMake build file           	  #
#                                                 #
#                                                 #
###################################################
cmake_minimum_required( VERSION 3.6 )

# function to collect all the sources from sub-directories
# into a single list
function(add_sources)
  get_property(is_defined GLOBAL PROPERTY SRCS_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY SRCS_LIST
      BRIEF_DOCS "List of source files"
      FULL_DOCS "List of source files to be compiled in one library")
  endif()
  # make absolute paths
  set(SRCS)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND SRCS "${s}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY SRCS_LIST "${SRCS}")
endfunction(add_sources)

# preprocess sources
function( preprocess_sources )
	set(PREP_SRCS PARENT_SCOPE)
	get_property(SRCS GLOBAL PROPERTY SRCS_LIST)
	foreach(s IN LISTS SRCS)
	  file(RELATIVE_PATH rs "${CMAKE_CURRENT_SOURCE_DIR}" "${s}")
	  string(REGEX REPLACE "r$" "" o "${CMAKE_CURRENT_BINARY_DIR}/${rs}")
	  add_custom_command(
		OUTPUT "${o}"
		COMMAND ${CMAKE_COMMAND} -E copy "${s}" "${o}"
		DEPENDS "${s}"
		COMMENT "Creating ${o}"
		VERBATIM
		)
	  list(APPEND PREP_SRCS "${s}")
	endforeach()
	set(PREP_SRCS ${PREP_SRCS} PARENT_SCOPE)
endfunction( preprocess_sources )

function( create_source_groups _src_root_path )
	get_property(SRCS GLOBAL PROPERTY SRCS_LIST)
	foreach(_source IN ITEMS ${SRCS})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()
endfunction( create_source_groups )

#Function to clear the sources list.
function( clear_sources )
	set_property( GLOBAL PROPERTY SRCS_LIST "" )
	set( PREP_SRCS PARENT_SCOPE )
endfunction( clear_sources )

# Function to add include files.
function( add_includes )
  get_property(is_defined GLOBAL PROPERTY INCLUDES_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY INCLUDES_LIST
      BRIEF_DOCS "List of include files"
      FULL_DOCS "List of include files to be compiled in one library")
  endif()
  # make absolute paths
  set(INCLUDES)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND INCLUDES "${s}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY INCLUDES_LIST "${INCLUDES}")
endfunction( add_includes )

# Function to install includes.
function( install_includes _include_root_path )
	get_property(INCLUDES GLOBAL PROPERTY INCLUDES_LIST)
	
	foreach(_include IN ITEMS ${INCLUDES})
		get_filename_component(_include_path "${_include}" PATH)
		file(RELATIVE_PATH _include_path_rel "${_include_root_path}" "${_include_path}")
		string(REPLACE "/" "\\" _group_path "${_include_path_rel}")
		install( FILES "${_include}" DESTINATION "include/${_group_path}" )
	endforeach()
	
	set_property( GLOBAL PROPERTY INCLUDES_LIST "" )
endfunction( install_includes )

macro(configure_msvc_runtime)
  if(MSVC)
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    # Set compiler options.
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${MSVC_RUNTIME} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

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

#Needs static CRT to avoid alloc issues.
set( MSVC_RUNTIME "static" )

configure_msvc_runtime()

set(variables
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
message(STATUS "Initial build flags:")
foreach(variable ${variables})
  message(STATUS "  '${variable}': ${${variable}}")
endforeach()
message(STATUS "")

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