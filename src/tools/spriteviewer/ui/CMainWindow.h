#ifndef SPRITEVIEWER_UI_CMAINWINDOW_H
#define SPRITEVIEWER_UI_CMAINWINDOW_H

#include "wxSpriteViewer.h"

#include "../settings/CSpriteViewerSettings.h"

namespace sprview
{
class CSpriteViewer;
class CMainPanel;

class CMainWindow final : public wxFrame
{
public:
	CMainWindow( CSpriteViewer* const pSpriteViewer );
	~CMainWindow();

	const CSpriteViewer* GetSpriteViewer() const { return m_pSpriteViewer; }
	CSpriteViewer* GetSpriteViewer() { return m_pSpriteViewer; }

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

	void RefreshRecentFiles();

	void LoadSprite( wxCommandEvent& event );
	void LoadBackgroundTexture( wxCommandEvent& event );
	void SaveSprite( wxCommandEvent& event );
	void OpenRecentFile( wxCommandEvent& event );
	void OnExit( wxCommandEvent& event );

	void TakeScreenshot( wxCommandEvent& event );
	void DumpSpriteInfo( wxCommandEvent& event );

	void ShowMessagesWindow( wxCommandEvent& event );

	void OnAbout( wxCommandEvent& event );

	void OnMessagesWindowClosed( wxCloseEvent& event );

private:
	CSpriteViewer* m_pSpriteViewer;
	CMainPanel* m_pMainPanel;

	wxMenuItem* m_RecentFiles[ CSpriteViewerSettings::MAX_RECENT_FILES ];

private:
	CMainWindow( const CMainWindow& ) = delete;
	CMainWindow& operator=( const CMainWindow& ) = delete;
};
}

#endif //SPRITEVIEWER_UI_CMAINWINDOW_H