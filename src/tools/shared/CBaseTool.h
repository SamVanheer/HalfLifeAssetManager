#ifndef TOOLS_SHARED_H
#define TOOLS_SHARED_H

#include <cstdint>

#include "ui/wxInclude.h"

#include "ui/utility/CTimer.h"

#include "ui/utility/IWindowCloseListener.h"

#include "settings/CBaseSettings.h"

class wxGLAttributes;
class wxGLContextAttrs;

namespace ui
{
class CMessagesWindow;
}

namespace tools
{
/**
*	Base class for tools. Defines common initialization and shutdown operations, and manages shared behavior.
*	Abstract.
*/
class CBaseTool : public ITimerListener, public IWindowCloseListener, public settings::ISettingsListener
{
public:
	static const size_t DEFAULT_MAX_MESSAGES_COUNT = 100;

protected:
	//TODO: remove these
	enum InitFlag
	{
		INIT_FILESYSTEM = 1 << 0,
		INIT_SOUNDSYSTEM = 1 << 1,
		INIT_OPENGL = 1 << 2,
		INIT_IMAGEHANDLERS = 1 << 3,

		INIT_ALL = INIT_FILESYSTEM | INIT_SOUNDSYSTEM | INIT_OPENGL | INIT_IMAGEHANDLERS
	};

	typedef uint8_t InitFlags_t;

protected:
	/**
	*	Constructor.
	*	@param initFlags Bit vector containing which systems to initialize, or INIT_ALL.
	*	@param szDisplayName Display name.
	*/
	CBaseTool( const InitFlags_t initFlags, const wxString szDisplayName, const wxIcon& toolIcon, settings::CBaseSettings* const pSettings );

public:
	virtual ~CBaseTool() = 0;

private:
	virtual void OnTimer( CTimer& timer ) override;

	virtual void OnWindowClose( wxFrame* pWindow, wxCloseEvent& event ) override;

	virtual void FPSChanged( const double flOldFPS, const double flNewFPS ) override;

public:
	/**
	*	Gets the settings object.
	*/
	const settings::CBaseSettings* GetSettings() const { return m_pSettings; }

	/**
	*	@copydoc GetSettings() const
	*/
	settings::CBaseSettings* GetSettings() { return m_pSettings; }

	/**
	*	Initializes the tool.
	*	@return true on success, false otherwise.
	*/
	bool Initialize();

	/**
	*	Shuts down the tool. This must be called even if Initialize returned false.
	*/
	void Shutdown();

	/**
	*	Runs a single frame for this tool.
	*/
	void ToolRunFrame();

	/**
	*	Returns whether the tool is exiting.
	*/
	bool IsExiting() const { return m_bExiting; }

	/**
	*	Makes the tool exit.
	*/
	void Exit( const bool bMainWndClosed = false );

	/**
	*	Gets the tool display name, if any.
	*/
	const wxString& GetDisplayName() const { return m_szDisplayName; }

	/**
	*	Gets the tool icon, used for window icons.
	*/
	const wxIcon& GetToolIcon() const { return m_ToolIcon; }

	/**
	*	Gets the log filename.
	*/
	const wxString& GetLogFileName() const;

	/**
	*	Sets the log filename. If empty, uses the display name.
	*/
	void SetLogFileName( const wxString& szFileName );

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
	/**
	*	Starts the render loop timer.
	*/
	void StartTimer( double flFPS );

	/**
	*	Allows an app to enable the messages window. This is a separate window containing log messages.
	*	bUse Whether to use the messages window or not.
	*/
	void UseMessagesWindow( const bool bUse );

	/**
	*	Allows the tool to configure custom canvas attributes.
	*/
	virtual void GetGLCanvasAttributes( wxGLAttributes& attrs );

	/**
	*	Allow the tool to configure custom context attributes.
	*/
	virtual void GetGLContextAttributes( wxGLContextAttrs& attrs );

	/**
	*	Called after the tool has been initialized.
	*	@return true on success, false otherwise.
	*/
	virtual bool PostInitialize() = 0;

	/**
	*	Called before the tool is shut down.
	*/
	virtual void PreShutdown() = 0;

	/**
	*	Called every frame.
	*/
	virtual void RunFrame() = 0;

	/**
	*	Called when the tool wants to exit.
	*/
	virtual void OnExit( const bool bMainWndClosed ) = 0;

private:
	void MessagesWindowClosed();

private:
	InitFlags_t m_InitFlags;
	const wxString m_szDisplayName;
	const wxIcon m_ToolIcon;

	wxString m_szLogFileName;

	settings::CBaseSettings* m_pSettings;

	CTimer* m_pTimer = nullptr;

	ui::CMessagesWindow* m_pMessagesWindow = nullptr;

	size_t m_uiMaxMessagesCount = DEFAULT_MAX_MESSAGES_COUNT;

	bool m_bExiting = false;

private:
	CBaseTool( const CBaseTool& ) = delete;
	CBaseTool& operator=( const CBaseTool& ) = delete;
};
}

#endif //TOOLS_SHARED_H