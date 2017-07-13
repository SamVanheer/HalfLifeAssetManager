if( MSVCRUNTIME_INCLUDED )
  return()
endif()
set( MSVCRUNTIME_INCLUDED true )

#!	Configures the runtime for the given target
#	Only one runtime can be provided
#
#	\arg:target_name Name of the target whose runtime to configure
#	\flag:STATIC Whether to use the static runtime
#	\flag:DYNAMIC Whether to use the dynamic runtime
#
macro( configure_msvc_runtime target_name )
	cmake_parse_arguments( ARG "STATIC;DYNAMIC" "" "" ${ARGN} )
	
	if( ARG_UNPARSED_ARGUMENTS )
		message( FATAL_ERROR "unrecognized arguments: ${ARG_UNPARSED_ARGUMENTS}" )
	endif()
	
	if( ARG_STATIC AND ARG_DYNAMIC )
		message( FATAL_ERROR "The STATIC and DYNAMIC runtimes are mutually exclusive" )
	endif()
	
	if( MSVC )
		# Set compiler options.
		if( ARG_STATIC )
			target_compile_options( ${target_name} PRIVATE $<$<CONFIG:DEBUG>:/MTd> )
			target_compile_options( ${target_name} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/MT> )
		elseif( ARG_DYNAMIC )
			target_compile_options( ${target_name} PRIVATE $<$<CONFIG:DEBUG>:/MDd> )
			target_compile_options( ${target_name} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/MD> )
		else()
			#Should never happen, but if more combinations are needed, this will cover edge cases.
			message( FATAL_ERROR "Unknown runtime selected" )
		endif()
	endif()
endmacro()
