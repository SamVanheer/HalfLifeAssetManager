#ifndef WXINCLUDE_H
#define WXINCLUDE_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

enum wxCommonIdentifier
{
	wxID_COMMON_LOWEST = wxID_HIGHEST + 1,

	//Messages window
	wxID_COMMON_MESSAGES_CLEAR,

	wxID_COMMON_HIGHEST
};

#endif //WXINCLUDE_H