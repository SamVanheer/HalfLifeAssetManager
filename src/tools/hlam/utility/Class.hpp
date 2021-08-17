#pragma once

/**
*	@brief This macro defines ThisClass to be the current class. Used in a class declaration for a class with no base.
*/
#define DECLARE_CLASS_NOBASE( thisClass )	\
using ThisClass = thisClass

/**
*	@brief This macro defines ThisClass to be the current class, and BaseClass to be the base class. Used in a class declaration.
*/
#define DECLARE_CLASS( thisClass, baseClass )	\
DECLARE_CLASS_NOBASE( thisClass );				\
using BaseClass = baseClass
