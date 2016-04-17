#include "Credits.h"

namespace tools
{
wxString GetSharedCredits()
{
	return wxString::Format( 
		"Contains FMOD, Copyright © Firelight Technologies Pty, Ltd., 2012-2016.\n\n"
		"Contains The OpenGL Extension Wrangler Library\n"
		"Copyright( C ) 2008 - 2016, Nigel Stewart <nigels[]users sourceforge net>\n"
		"Copyright( C ) 2002 - 2008, Milan Ikits <milan ikits[]ieee org>\n"
		"Copyright( C ) 2002 - 2008, Marcelo E.Magallon <mmagallo[]debian org>\n"
		"Copyright( C ) 2002, Lev Povalahev\n"
		"All rights reserved.\n\n"
		"Uses wxWidgets %d.%d.%d\n\n"
		"Build Date: %s\n",
		wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER,
		__DATE__
	);
}
}