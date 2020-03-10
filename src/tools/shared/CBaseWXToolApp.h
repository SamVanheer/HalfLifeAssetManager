#ifndef TOOLS_SHARED_CBASEWXTOOLAPP_H
#define TOOLS_SHARED_CBASEWXTOOLAPP_H

#include "ui/wxHLMV.h"

#include "ui/wx/CwxOpenGL.h"

#include "ui/wx/utility/IWindowCloseListener.h"

#include "CBaseToolApp.h"

namespace ui
{
class CMessagesWindow;
}

namespace tools
{
class CBaseWXToolApp : public CBaseToolApp, public wxApp, public IWindowCloseListener
{
public:
	static const size_t DEFAULT_MAX_MESSAGES_COUNT = 100;

public:

protected:
	bool Initialize() override;

	bool InitOpenGL() override;

	void ShutdownOpenGL() override;

	/**
	*	Allows the tool to configure custom canvas attributes.
	*/
	virtual void GetGLCanvasAttributes( wxGLAttributes& attrs );

	/**
	*	Allow the tool to configure custom context attributes.
	*/
	virtual void GetGLContextAttributes( wxGLContextAttrs& attrs );

	/**
	*	Allows wxWidgets apps to run code after base tool app run code has been executed, but before the wxWidgets app is started.
	*	@return true on success, false otherwise.
	*/
	virtual bool PreRunApp() { return true; }

public:
	//wxApp overrides

	bool OnInit() override;

	int OnExit() override;

	/**
	*	Returns whether the tool is exiting.
	*/
	bool IsExiting() const { return m_bExiting; }

	/**
	*	Makes the tool exit.
	*/
	void Exit( const bool bMainWndClosed = false );

protected:
	/**
	*	Called every frame.
	*/
	virtual void RunFrame() = 0;

	/**
	*	Called when the tool wants to exit.
	*/
	virtual void OnExit( const bool bMainWndClosed ) = 0;

public:
	const wxIcon& GetToolIcon() const { return m_ToolIcon; }

	void SetToolIcon( const wxIcon& icon )
	{
		m_ToolIcon = icon;
	}

	/**
	*	Returns the messages window, if it exists.
	*/
	ui::CMessagesWindow* GetMessagesWindow() { return m_pMessagesWindow; }

	/**
	*	Returns whether the messages window is visible.
	*/
	bool IsUsingMessagesWindow() const { return m_pMessagesWindow != nullptr; }

	/**
	*	Shows or hides the messages window.
	*/
	void ShowMessagesWindow( const bool bShow );

	/**
	*	Gets the maximum messages count.
	*/
	size_t GetMaxMessagesCount() const;

	/**
	*	Sets the maximum messages count.
	*/
	void SetMaxMessagesCount( const size_t uiMaxMessagesCount );

protected:
	void OnWindowClose( wxFrame* pWindow, wxCloseEvent& event ) override;

	/**
	*	Allows an app to enable the messages window. This is a separate window containing log messages.
	*	bUse Whether to use the messages window or not.
	*/
	void UseMessagesWindow( const bool bUse );

private:
	void MessagesWindowClosed();

protected:
	void OnIdle( wxIdleEvent& event );

private:
	bool m_bExiting = false;

	wxIcon m_ToolIcon;

	ui::CMessagesWindow* m_pMessagesWindow = nullptr;

	size_t m_uiMaxMessagesCount = DEFAULT_MAX_MESSAGES_COUNT;
};
}

#endif //TOOLS_SHARED_CBASEWXTOOLAPP_H