#ifndef SPRITEVIEWER_UI_CSPRITEVIEWERAPP_H
#define SPRITEVIEWER_UI_CSPRITEVIEWERAPP_H

#include "wxSpriteViewer.h"

#include "tools/shared/CBaseWXToolApp.h"

#include "../settings/CSpriteViewerSettings.h"
#include "../CSpriteViewerState.h"

namespace sprview
{
class CMainWindow;

class CSpriteViewerApp final : public tools::CBaseWXToolApp
{
public:
	bool OnInit() override;

	virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;

	virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

	const CSpriteViewerState* GetState() const { return m_pState; }
	CSpriteViewerState* GetState() { return m_pState; }

	const CSpriteViewerSettings* GetSettings() const { return m_pSettings; }
	CSpriteViewerSettings* GetSettings() { return m_pSettings; }

	/**
	*	Gets the main window.
	*/
	CMainWindow* GetMainWindow() { return m_pMainWindow; }

	/**
	*	Sets the main window.
	*/
	void SetMainWindow( CMainWindow* const pMainWindow );

	bool LoadSprite( const wxString& szFilename );

protected:
	bool Connect( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	bool PreRunApp() override;

	void ShutdownApp() override;

	void RunFrame() override;

	void OnExit( const bool bMainWndClosed ) override final;

private:
	CSpriteViewerState* m_pState = nullptr;
	CSpriteViewerSettings* m_pSettings = nullptr;
	sprview::CMainWindow* m_pMainWindow = nullptr;

	wxString m_szSprite;		//Sprite to load on startup, if any.
};
}

wxDECLARE_APP( sprview::CSpriteViewerApp );

#endif //SPRITEVIEWER_UI_CSPRITEVIEWERAPP_H