#ifndef SPRITEVIEWER_UI_CMAINWINDOW_H
#define SPRITEVIEWER_UI_CMAINWINDOW_H

#include "wxSpriteViewer.h"

#include "../settings/CSpriteViewerSettings.h"

#include "ui/wx/utility/CwxRecentFiles.h"
#include "ui/wx/shared/CwxBaseFrame.h"

namespace sprview
{
class CSpriteViewerApp;
class CMainPanel;

class CMainWindow final : public ui::CwxBaseFrame
{
public:
	CMainWindow( CSpriteViewerApp* const pSpriteViewer );
	~CMainWindow();

	const CSpriteViewerApp* GetSpriteViewer() const { return m_pSpriteViewer; }
	CSpriteViewerApp* GetSpriteViewer() { return m_pSpriteViewer; }

	void RunFrame();

	bool LoadSprite( const wxString& szFilename );
	bool PromptLoadSprite();

	bool SaveSprite( const wxString& szFilename );
	bool PromptSaveSprite();

	bool LoadBackgroundTexture( const wxString& szFilename );
	bool PromptLoadBackgroundTexture();

	void UnloadBackgroundTexture();

	void TakeScreenshot();

	void DumpSpriteInfo();

private:
	wxDECLARE_EVENT_TABLE();

	void LoadSprite( wxCommandEvent& event );
	void LoadBackgroundTexture( wxCommandEvent& event );
	void SaveSprite( wxCommandEvent& event );
	void OpenRecentFile( wxCommandEvent& event );
	void OnClose( wxCloseEvent& event );
	void OnExit( wxCommandEvent& event );

	void TakeScreenshot( wxCommandEvent& event );
	void DumpSpriteInfo( wxCommandEvent& event );

	void ShowMessagesWindow( wxCommandEvent& event );

	void OnAbout( wxCommandEvent& event );

	void OnMessagesWindowClosed( wxCloseEvent& event );

private:
	CSpriteViewerApp* m_pSpriteViewer;
	CMainPanel* m_pMainPanel;

	ui::CwxRecentFiles m_RecentFiles;

private:
	CMainWindow( const CMainWindow& ) = delete;
	CMainWindow& operator=( const CMainWindow& ) = delete;
};
}

#endif //SPRITEVIEWER_UI_CMAINWINDOW_H