#ifndef UI_CWXBASEAPP_H
#define UI_CWXBASEAPP_H

#include <cstdint>

#include "wxInclude.h"

#include "ui/utility/CTimer.h"

#include "ui/utility/IWindowCloseListener.h"

class CMessagesWindow;

class CwxBaseApp : public wxApp, public ITimerListener, public IWindowCloseListener
{
public:
	static const size_t DEFAULT_MAX_MESSAGES_COUNT = 100;

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

	virtual void OnWindowClose( wxFrame* pWindow, wxCloseEvent& event ) override;

	bool IsExiting() const { return m_bExiting; }

	virtual void ExitApp( const bool bMainWndClosed = false );

	CMessagesWindow* GetMessagesWindow() { return m_pMessagesWindow; }

	bool IsUsingMessagesWindow() const { return m_pMessagesWindow != nullptr; }

	void ShowMessagesWindow( const bool bShow );

	size_t GetMaxMessagesCount() const { return m_uiMaxMessagesCount; }

	void SetMaxMessagesCount( const size_t uiMaxMessagesCount );

protected:
	bool InitApp( InitFlags_t initFlags, const wxString& szDisplayName = "" );
	void ShutdownApp();

	void StartTimer();

	/**
	*	Allows an app to enable the messages window. This is a separate window containing log messages.
	*	bUse Whether to use the messages window or not.
	*/
	void UseMessagesWindow( const bool bUse );

private:
	void MessagesWindowClosed();

private:
	CTimer* m_pTimer = nullptr;

	CMessagesWindow* m_pMessagesWindow = nullptr;

	size_t m_uiMaxMessagesCount = DEFAULT_MAX_MESSAGES_COUNT;

	bool m_bExiting = false;

private:
	CwxBaseApp( const CwxBaseApp& ) = delete;
	CwxBaseApp& operator=( const CwxBaseApp& ) = delete;
};

#endif //UI_CWXBASEAPP_H