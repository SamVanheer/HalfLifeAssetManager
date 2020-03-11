#ifndef CMODELVIEWERAPP_H
#define CMODELVIEWERAPP_H

#include <string>

#include "wxHLMV.h"

#include <wx/app.h>

#include "../CHLMVState.h"
#include "../settings/CHLMVSettings.h"

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

namespace hlmv
{
class CMainWindow;
class CFullscreenWindow;

class CModelViewerApp : public wxApp
{
public:
	static const size_t DEFAULT_MAX_MESSAGES_COUNT = 100;

	bool OnInit() override;

	int OnExit() override;

	void OnInitCmdLine( wxCmdLineParser& parser ) override;

	bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

	CHLMVState* GetState() { return m_pState; }

	CHLMVSettings* GetSettings() { return m_pSettings; }

	CMainWindow* GetMainWindow() { return m_pMainWindow; }

	void SetMainWindow( CMainWindow* const pMainWindow )
	{
		m_pMainWindow = pMainWindow;
	}

	CFullscreenWindow* GetFullscreenWindow() { return m_pFullscreenWindow; }

	void SetFullscreenWindow( CFullscreenWindow* const pWindow )
	{
		m_pFullscreenWindow = pWindow;
	}

	void Exit(const bool bMainWndClosed = false);

private:
	bool Startup();

	void Shutdown();

	void RunFrame();

	void OnIdle(wxIdleEvent& event);

	void OnWindowClose(wxCloseEvent& event);

	/**
	*	Allows an app to enable the messages window. This is a separate window containing log messages.
	*	bUse Whether to use the messages window or not.
	*/
	void UseMessagesWindow(const bool bUse);

	void MessagesWindowClosed();

public:
	const wxIcon& GetToolIcon() const { return m_ToolIcon; }

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
	void ShowMessagesWindow(const bool bShow);

	/**
	*	Gets the maximum messages count.
	*/
	size_t GetMaxMessagesCount() const;

	/**
	*	Sets the maximum messages count.
	*/
	void SetMaxMessagesCount(const size_t uiMaxMessagesCount);

	//Load/Save model
	/**
	*	Loads a model with the given filename.
	*	@param pszFilename Model filename.
	*	@return true if the model was successfully loaded, false otherwise.
	*/
	bool LoadModel( const char* const pszFilename );

	bool PromptLoadModel();

	/**
	*	Saves the current model with the given filename.
	*	@param pszFilename Filename to save the model as.
	*	@return true if a model is currently loaded, and the model was successfully saved.
	*/
	bool SaveModel( const char* const pszFilename );

	bool PromptSaveModel();

	//Background and Ground textures
	/**
	*	Loads a background texture, replacing the current background texture, if one is present.
	*	@param pszFilename Filename of the texture to load.
	*	@return true if the texture was successfully loaded, false otherwise.
	*/
	bool LoadBackgroundTexture( const char* const pszFilename );

	bool PromptLoadBackgroundTexture();

	void UnloadBackgroundTexture();

	/**
	*	Loads a ground texture, replacing the current ground texture, if one is present.
	*	@param pszFilename Filename of the texture to load.
	*	@return true if the texture was successfully loaded, false otherwise.
	*/
	bool LoadGroundTexture( const char* const pszFilename );

	bool PromptGroundTexture();

	void UnloadGroundTexture();

	void SaveUVMap( const wxString& szFilename, const int iTexture );

private:
	filesystem::IFileSystem* m_pFileSystem = nullptr;
	soundsystem::ISoundSystem* m_pSoundSystem = nullptr;

	bool m_bExiting = false;

	wxIcon m_ToolIcon;

	ui::CMessagesWindow* m_pMessagesWindow = nullptr;

	size_t m_uiMaxMessagesCount = DEFAULT_MAX_MESSAGES_COUNT;

	CHLMVState* m_pState = nullptr;
	CHLMVSettings* m_pSettings = nullptr;

	CMainWindow* m_pMainWindow = nullptr;
	CFullscreenWindow* m_pFullscreenWindow = nullptr;

	wxString m_szModel;		//Model to load on startup, if any.
};
}

wxDECLARE_APP( hlmv::CModelViewerApp );

#endif //CMODELVIEWERAPP_H