#ifndef HLMV_UI_MOUSEOPFLAG_H
#define HLMV_UI_MOUSEOPFLAG_H

#include "shared/Utility.h"

namespace hlmv
{
typedef unsigned int MouseOpFlags_t;

/**
*	Flags used to inform the 3D view of which mouse operations are allowed by which control panels.
*/
enum MouseOpFlag : MouseOpFlags_t
{
	/**
	*	No operations are allowed.
	*/
	MOUSEOPF_NONE			= 0,

	/**
	*	Allow translation in 3D space.
	*/
	MOUSEOPF_TRANSLATE		= Bit( 0 ),

	/**
	*	Allow rotation in 3D space.
	*/
	MOUSEOPF_ROTATE			= Bit( 1 ),

	/**
	*	Allow the light vector to be modified.
	*/
	MOUSEOPF_LIGHTVECTOR	= Bit( 2 ),

	/**
	*	Allow all operations.
	*/
	MOUSEOPF_ALL = MOUSEOPF_TRANSLATE | MOUSEOPF_ROTATE | MOUSEOPF_LIGHTVECTOR
};
}

#endif //HLMV_UI_MOUSEOPFLAG_H