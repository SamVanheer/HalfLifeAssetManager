if( WINXPSUPPORT_INCLUDED )
  return()
endif()
set( WINXPSUPPORT_INCLUDED true )

#!	Checks if thread-safe initialization needs to be disabled for the given target.
#	If targeting Windows XP (v*_xp toolset) and using an MSVC version of Visual Studio 2015 or newer, it is disabled.
#	\arg:target_name The name of the target to check
macro( check_winxp_support target_name )
	#Visual Studio only.
	if( MSVC )
		#Verify that we have the data we need.
		if( NOT TARGET ${target_name} )
			message( FATAL_ERROR "Target ${target_name} does not exist" )
		endif()

		#VS2015 and newer requires this
		if( NOT "${MSVC_VERSION}" LESS 1900 AND CMAKE_VS_PLATFORM_TOOLSET MATCHES ".*_xp$" )
			get_target_property( FLAGS ${target_name} COMPILE_FLAGS )
			
			#Check if it's already set. Eliminates some noise in the output if it was explicitly disabled before
			if( ${FLAGS} STREQUAL "NOTFOUND" )
				message( WARNING "Couldn't get compile flags from target ${target_name}" )
			else()
				if( NOT FLAGS MATCHES "/Zc:threadSafeInit-" )
					message( STATUS "${target_name}: Disabling Thread-safe initialization for Windows XP support" )
					#Disable thread-safe init so Windows XP users don't get crashes.

					target_compile_options( ${target_name} PRIVATE
						/Zc:threadSafeInit-
					)
				endif()
			endif()
		endif()
	endif()
endmacro( check_winxp_support )
