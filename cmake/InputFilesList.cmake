if( INPUTFILESLIST_INCLUDED )
  return()
endif()
set( INPUTFILESLIST_INCLUDED true )

#
#	This file defines functions to build a list of source and include files to be used to create libraries and executables, and install files, respectively.
#
#	Call add_sources with one or more source/header files to add them to the list of files.
#	Call preprocess_sources to generate a list of files to include in a library or executable.
#	Use the PREP_SRCS variable to include the list.
#	Call create_source_groups with the root directory for the files in the list after defining the library or executable to generate filters for Visual Studio that match the directory structure.
#	Call add_includes to add files to the list of files to install.
#	Call install_includes with the root directory for the files in the list after defining the library or executable to install the included files.
#	Call clear_sources to clear the sources list and prepare it for the next library or executable.
#

#! Gets the list of source files
#	\arg:dest_var Name of the variable that will contain the list
function( get_sources dest_var )
	get_property( is_defined GLOBAL PROPERTY SRCS_LIST DEFINED )
	
	if( is_defined )
		get_property( srcs GLOBAL PROPERTY SRCS_LIST )
		set( ${dest_var} ${srcs} PARENT_SCOPE )
	else()
		set( ${dest_var} PARENT_SCOPE )
	endif()
endfunction( get_sources )

#!	Function to collect all the sources from sub-directories into a single list
#	Pass the file paths of source files relative to the CMakeLists that is calling this function
function( add_sources )
	get_property( is_defined GLOBAL PROPERTY SRCS_LIST DEFINED )
	
	if( NOT is_defined )
		define_property( GLOBAL PROPERTY SRCS_LIST
			BRIEF_DOCS "List of source files"
			FULL_DOCS "List of source files to be compiled in one library"
		)
	endif()
	
	# make absolute paths
	set( SRCS )
	foreach( s IN LISTS ARGN )
		if( NOT IS_ABSOLUTE "${s}" )
			get_filename_component( s "${s}" ABSOLUTE )
		endif()
		list( APPEND SRCS "${s}" )
	endforeach()
	
	# append to global list
	set_property( GLOBAL APPEND PROPERTY SRCS_LIST "${SRCS}" )
endfunction( add_sources )

#!	Preprocess source files
function( preprocess_sources )
	set( PREP_SRCS PARENT_SCOPE )
	get_property( SRCS GLOBAL PROPERTY SRCS_LIST )
	
	foreach( s IN LISTS SRCS )
		file( RELATIVE_PATH rs "${CMAKE_CURRENT_SOURCE_DIR}" "${s}" )
		string( REGEX REPLACE "r$" "" o "${CMAKE_CURRENT_BINARY_DIR}/${rs}" )
		add_custom_command(
			OUTPUT "${o}"
			COMMAND ${CMAKE_COMMAND} -E copy "${s}" "${o}"
			DEPENDS "${s}"
			COMMENT "Creating ${o}"
			VERBATIM
		)
		list( APPEND PREP_SRCS "${s}" )
	endforeach()
	
	set( PREP_SRCS ${PREP_SRCS} PARENT_SCOPE )
endfunction( preprocess_sources )

#!	Create the source groups for source files
#	\arg:_src_root_path Root directory for the list of source files
function( create_source_groups _src_root_path )
	get_property( SRCS GLOBAL PROPERTY SRCS_LIST )
	
	foreach( _source IN ITEMS ${SRCS} )
		get_filename_component( _source_path "${_source}" PATH )
		file( RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}" )
		string( REPLACE "/" "\\" _group_path "${_source_path_rel}" )
		source_group( "${_group_path}" FILES "${_source}" )
	endforeach()
endfunction( create_source_groups )

#!	Function to clear the sources list
#	Call once sources have been added to a target and source groups have been created
function( clear_sources )
	set_property( GLOBAL PROPERTY SRCS_LIST "" )
	set( PREP_SRCS PARENT_SCOPE )
endfunction( clear_sources )

#!	Function to add include files to a single list
function( add_includes )
	get_property( is_defined GLOBAL PROPERTY INCLUDES_LIST DEFINED )
	if( NOT is_defined )
		define_property( GLOBAL PROPERTY INCLUDES_LIST
			BRIEF_DOCS "List of include files"
			FULL_DOCS "List of include files to be compiled in one library"
		)
	endif()
	
	# make absolute paths
	set( INCLUDES )
	foreach( s IN LISTS ARGN )
		if( NOT IS_ABSOLUTE "${s}" )
			get_filename_component( s "${s}" ABSOLUTE )
		endif()
		list( APPEND INCLUDES "${s}" )
	endforeach()
	
	# append to global list
	set_property( GLOBAL APPEND PROPERTY INCLUDES_LIST "${INCLUDES}" )
endfunction( add_includes )

#!	Function to install includes
#	\arg:_include_root_path Root directory for the list of include files
function( install_includes _include_root_path )
	get_property( INCLUDES GLOBAL PROPERTY INCLUDES_LIST )
	
	foreach( _include IN ITEMS ${INCLUDES} )
		get_filename_component( _include_path "${_include}" PATH )
		file( RELATIVE_PATH _include_path_rel "${_include_root_path}" "${_include_path}" )
		string( REPLACE "/" "\\" _group_path "${_include_path_rel}" )
		install( FILES "${_include}" DESTINATION "include/${_group_path}" )
	endforeach()
	
	set_property( GLOBAL PROPERTY INCLUDES_LIST "" )
endfunction( install_includes )
