if( PDBUTILS_INCLUDED )
  return()
endif()
set( PDBUTILS_INCLUDED true )

#!	Sets PDB file names to match the binaries
#	\arg:target_name Name of the target to configure PDB names for
macro( set_pdb_names target_name )
	foreach( config IN LISTS CMAKE_CONFIGURATION_TYPES )
			#TODO: figure out if there's a way to check if a config is debug - Solokiller
			set( POSTFIX )
			if( ${config} STREQUAL "Debug" )
				set( POSTFIX ${CMAKE_DEBUG_POSTFIX} )
			endif()
			
			string( TOUPPER ${config} PDB_POSTFIX )
			
			set_target_properties( ${target_name} PROPERTIES COMPILE_PDB_NAME_${PDB_POSTFIX} "${target_name}${POSTFIX}" )
		endforeach()
endmacro( set_pdb_names )

#!	Installs PDB files for a given target
#	\arg:target_name Name of the target to install PDB files for
#	\arg:destination Destination to install files to
macro( install_pdbs target_name destination )
	if( WIN32 )
		foreach( config IN LISTS CMAKE_CONFIGURATION_TYPES )
			#TODO: figure out if there's a way to check if a config is debug - Solokiller
			set( POSTFIX )
			if( ${config} STREQUAL "Debug" )
				set( POSTFIX ${CMAKE_DEBUG_POSTFIX} )
			endif()
			
			install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${config}/${target_name}${POSTFIX}.pdb DESTINATION ${destination} CONFIGURATIONS ${config} )
		endforeach()
	endif()
endmacro( install_pdbs )
