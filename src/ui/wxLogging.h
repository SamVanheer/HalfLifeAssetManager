#ifndef UI_LOGGING_H
#define UI_LOGGING_H

#include "shared/Logging.h"

class CwxDefaultLogListener final : public ILogListener
{
public:
	void LogMessage( const LogType type, const char* const pszMessage ) override final;
};

ILogListener* GetwxDefaultLogListener();

#endif //UI_LOGGING_H