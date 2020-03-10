#ifndef COMMON_CLASS_H
#define COMMON_CLASS_H

//wxWidgets defines these, so undefine them.
#ifdef DECLARE_CLASS_NOBASE
#undef DECLARE_CLASS_NOBASE
#endif

#ifdef DECLARE_CLASS
#undef DECLARE_CLASS
#endif

/**
*	This macro defines ThisClass to be the current class. Used in a class declaration for a class with no base.
*/
#define DECLARE_CLASS_NOBASE( thisClass )	\
typedef thisClass ThisClass

/**
*	This macro defines ThisClass to be the current class, and BaseClass to be the base class. Used in a class declaration.
*/
#define DECLARE_CLASS( thisClass, baseClass )	\
DECLARE_CLASS_NOBASE( thisClass );				\
typedef baseClass BaseClass

#endif //COMMON_CLASS_H