#ifndef TOOLS_SHARED_H
#define TOOLS_SHARED_H

#include <cstdint>

#include "ui/wx/wxInclude.h"

#include "ui/wx/utility/CTimer.h"

#include "ui/wx/utility/IWindowCloseListener.h"

#include "settings/CBaseSettings.h"

#include "lib/CLibrary.h"

class wxGLAttributes;
class wxGLContextAttrs;

class ILibSystem;

namespace filesystem
{
class IFileSystem;
}

namespace soundsystem
{
class ISoundSystem;
}

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
	/**
	*	Constructor.
	*	@param szDisplayName Display name.
	*	@param toolIcon The icon to use for windows.
	*/
	CBaseTool( const wxString szDisplayName, const wxIcon& toolIcon );

public:
	virtual ~CBaseTool() = 0;

private:
	virtual void OnTimer( CTimer& timer ) override;

	virtual void OnWindowClose( wxFrame* pWindow, wxCloseEvent& event ) override;

	virtual void FPSChanged( const double flOldFPS, const double flNewFPS ) override;

public:
	/**
	*	Gets the file system.
	*/
	filesystem::IFileSystem* GetFileSystem() const { return m_pFileSystem; }

	/**
	*	Gets the sound system.
	*/
	soundsystem::ISoundSystem* GetSoundSystem() const { return m_pSoundSystem; }

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
	*	Returns whether the timer is running yet.
	*/
	bool IsTimerRunning() const;

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
	*	Creates the settings instance. This is called once all tool system initialization has completed, allowing the settings object to use systems.
	*/
	virtual settings::CBaseSettings* CreateSettings() = 0;

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
	const wxString m_szDisplayName;
	const wxIcon m_ToolIcon;

	wxString m_szLogFileName;

	CLibrary m_CVarSystemLib;
	CLibrary m_FileSystemLib;
	CLibrary m_RendererLib;
	ILibSystem* m_pRendererLib = nullptr;
	CLibrary m_SoundSystemLib;

	filesystem::IFileSystem* m_pFileSystem = nullptr;
	soundsystem::ISoundSystem* m_pSoundSystem = nullptr;

	settings::CBaseSettings* m_pSettings = nullptr;

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