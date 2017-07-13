if( TERNARY_INCLUDED )
  return()
endif()
set( TERNARY_INCLUDED true )

#!	Ternary operator
#
#	\arg:result_var_name Name of the variable to assign the value to
#	\arg:condition Condition to evaluate
#	\arg:value_if_true Value to set if the condition is true
#	\arg:value_if_false Value to set if the condition is false
macro( ternary result_var_name condition value_if_true value_if_false )
	if( ${condition} )
		set( ${result_var_name} ${value_if_true} )
	else()
		set( ${result_var_name} ${value_if_false} )
	endif()
endmacro( ternary )
