#include "wxInclude.h"

#include <cstdarg>
#include <cstring>

#include "wxLogging.h"

void CwxDefaultLogListener::LogMessage( const LogType type, const char* const pszMessage )
{
	wxString szCaption;
	long iStyle = 0;

	switch( type )
	{
	default:
	case LogType::MESSAGE:
		{
			szCaption = "Message";
			break;
		}

	case LogType::WARNING:
		{
			szCaption = "Warning";
			iStyle = wxICON_WARNING;
			break;
		}

	case LogType::ERROR:
		{
			szCaption = "Error";
			iStyle = wxICON_ERROR;
			break;
		}

	case LogType::FATAL_ERROR:
		{
			szCaption = "Fatal Error";
			iStyle = wxICON_ERROR;
			break;
		}
	}

	wxMessageBox( pszMessage, szCaption, wxOK | wxCENTRE | iStyle );
}

namespace
{
static CwxDefaultLogListener g_DefaultLogListener;
}

ILogListener* GetwxDefaultLogListener()
{
	return &g_DefaultLogListener;
}