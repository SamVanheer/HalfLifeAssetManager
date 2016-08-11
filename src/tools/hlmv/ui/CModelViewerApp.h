#ifndef CMODELVIEWERAPP_H
#define CMODELVIEWERAPP_H

#include "wxHLMV.h"

#include "tools/shared/CBaseWXToolApp.h"

#include "../CHLMVState.h"
#include "../settings/CHLMVSettings.h"

namespace hlmv
{
class CMainWindow;
class CFullscreenWindow;

class CModelViewerApp : public tools::CBaseWXToolApp
{
public:
	bool OnInit() override;

	virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;

	virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

	/**
	*	Gets the state object.
	*/
	const CHLMVState* GetState() const { return m_pState; }

	/**
	*	@copydoc GetState() const
	*/
	CHLMVState* GetState() { return m_pState; }

	/**
	*	Gets the settings object.
	*/
	const CHLMVSettings* GetSettings() const { return m_pSettings; }

	/**
	*	@copydoc GetSettings() const
	*/
	CHLMVSettings* GetSettings() { return m_pSettings; }

	/**
	*	Gets the main window.
	*/
	CMainWindow* GetMainWindow() { return m_pMainWindow; }

	/**
	*	Sets the main window.
	*/
	void SetMainWindow( CMainWindow* const pMainWindow );

	/**
	*	Gets the fullscreen window.
	*/
	CFullscreenWindow* GetFullscreenWindow() { return m_pFullscreenWindow; }

	/**
	*	Sets the fullscreen window.
	*/
	void SetFullscreenWindow( CFullscreenWindow* const pWindow );

protected:
	bool PreRunApp() override;

	void ShutdownApp() override;

	void RunFrame() override;

	void OnExit( const bool bMainWndClosed ) override final;

public:

	//Load/Save model
	/**
	*	Loads a model with the given filename.
	*	@param pszFilename Model filename.
	*	@return true if the model was successfully loaded, false otherwise.
	*/
	bool LoadModel( const char* const pszFilename );

	/**
	*	Prompts the user to load a model.
	*	@return true if the user elected to load a model, and the model was successfully loaded, false otherwise.
	*/
	bool PromptLoadModel();

	/**
	*	Saves the current model with the given filename.
	*	@param pszFilename Filename to save the model as.
	*	@return true if a model is currently loaded, and the model was successfully saved.
	*/
	bool SaveModel( const char* const pszFilename );

	/**
	*	Prompts the user to save the current model.
	*	@return true if the user elected to save the model, and the model was successfully saved, false otherwise.
	*/
	bool PromptSaveModel();

	//Background and Ground textures
	/**
	*	Loads a background texture, replacing the current background texture, if one is present.
	*	@param pszFilename Filename of the texture to load.
	*	@return true if the texture was successfully loaded, false otherwise.
	*/
	bool LoadBackgroundTexture( const char* const pszFilename );

	/**
	*	Prompts the user to load a background texture.
	*	@return true if the user elected to load a background texture, and the texture was successfully loaded, false otherwise.
	*/
	bool PromptLoadBackgroundTexture();

	/**
	*	Unloads the current background texture.
	*/
	void UnloadBackgroundTexture();

	/**
	*	Loads a ground texture, replacing the current ground texture, if one is present.
	*	@param pszFilename Filename of the texture to load.
	*	@return true if the texture was successfully loaded, false otherwise.
	*/
	bool LoadGroundTexture( const char* const pszFilename );

	/**
	*	Prompts the user to load a ground texture.
	*	@return true if the user elected to load a ground texture, and the texture was successfully loaded, false otherwise.
	*/
	bool PromptGroundTexture();

	/**
	*	Unloads the current ground texture.
	*/
	void UnloadGroundTexture();

	/**
	*	Saves the given texture's UV map to the given file.
	*/
	void SaveUVMap( const wxString& szFilename, const int iTexture );

private:
	CHLMVState* m_pState = nullptr;
	CHLMVSettings* m_pSettings = nullptr;

	CMainWindow* m_pMainWindow = nullptr;
	CFullscreenWindow* m_pFullscreenWindow = nullptr;

	wxString m_szModel;		//Model to load on startup, if any.
};
}

wxDECLARE_APP( hlmv::CModelViewerApp );

#endif //CMODELVIEWERAPP_H