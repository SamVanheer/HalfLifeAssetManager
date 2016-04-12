#ifndef UI_CWXBASEAPP_H
#define UI_CWXBASEAPP_H

#include <cstdint>

#include "wxInclude.h"

#include "ui/utility/CTimer.h"

class CwxBaseApp : public wxApp, public ITimerListener
{
protected:
	enum InitFlag
	{
		INIT_FILESYSTEM		= 1 << 0,
		INIT_SOUNDSYSTEM	= 1 << 1,
		INIT_OPENGL			= 1 << 2,
		INIT_IMAGEHANDLERS	= 1 << 3,

		INIT_ALL = INIT_FILESYSTEM | INIT_SOUNDSYSTEM | INIT_OPENGL | INIT_IMAGEHANDLERS
	};

	typedef uint8_t InitFlags_t;

public:
	CwxBaseApp();
	virtual ~CwxBaseApp() = 0;

	virtual void OnTimer( CTimer& timer ) override;

	virtual void ExitApp( const bool bMainWndClosed = false );

protected:
	bool InitApp( InitFlags_t initFlags, const wxString& szDisplayName = "" );
	void ShutdownApp();

	void StartTimer();

private:
	CTimer* m_pTimer = nullptr;

private:
	CwxBaseApp( const CwxBaseApp& ) = delete;
	CwxBaseApp& operator=( const CwxBaseApp& ) = delete;
};

#endif //UI_CWXBASEAPP_H