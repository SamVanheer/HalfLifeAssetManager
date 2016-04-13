#include "wxInclude.h"

#include <cstdarg>
#include <cstring>

#include "wxLogging.h"

namespace
{
class CDefaultLogListener final : public ILogListener
{
public:
	void LogMessage( const LogType type, const char* const pszMessage ) override final
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
		}

		wxMessageBox( pszMessage, szCaption, wxOK | wxCENTRE | iStyle );
	}
};

static CDefaultLogListener g_DefaultLogListener;
}

ILogListener* GetDefaultLogListener()
{
	return &g_DefaultLogListener;
}