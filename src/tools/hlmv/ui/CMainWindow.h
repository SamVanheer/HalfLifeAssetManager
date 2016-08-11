#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "wxHLMV.h"

#include <wx/dnd.h>

#include "../settings/CHLMVSettings.h"
#include "../CHLMVState.h"

#include "ui/wx/utility/CwxRecentFiles.h"
#include "ui/wx/shared/CwxBaseFrame.h"

namespace hlmv
{
class CModelViewerApp;
class CMainPanel;
class CMainWindow;

class CModelDropTarget final : public wxFileDropTarget
{
public:
	CModelDropTarget( CMainWindow* const pMainWindow )
		: m_pMainWindow( pMainWindow )
	{
	}

	bool OnDropFiles( wxCoord x, wxCoord y, const wxArrayString& filenames ) override final;

private:
	CMainWindow* const m_pMainWindow;
};

class CMainWindow final : public ui::CwxBaseFrame
{
public:
	CMainWindow( CModelViewerApp* const pHLMV );
	~CMainWindow();

	const CModelViewerApp* GetHLMV() const { return m_pHLMV; }
	CModelViewerApp* GetHLMV() { return m_pHLMV; }

	void RunFrame();

	bool LoadModel( const wxString& szFilename );
	bool PromptLoadModel();

	bool SaveModel( const wxString& szFilename );
	bool PromptSaveModel();

	bool LoadBackgroundTexture( const wxString& szFilename );
	bool PromptLoadBackgroundTexture();

	void UnloadBackgroundTexture();

	bool LoadGroundTexture( const wxString& szFilename );
	bool PromptLoadGroundTexture();

	void UnloadGroundTexture();

	void SaveUVMap( const wxString& szFilename, const int iTexture );

	void CenterView();

	void SaveView();

	void RestoreView();

	void TakeScreenshot();

	void DumpModelInfo();

	bool OnDropFiles( wxCoord x, wxCoord y, const wxArrayString& filenames );

private:
	wxDECLARE_EVENT_TABLE();

	bool ShowUnsavedWarning();

	void LoadModel( wxCommandEvent& event );
	void LoadBackgroundTexture( wxCommandEvent& event );
	void LoadGroundTexture( wxCommandEvent& event );
	void UnloadGroundTexture( wxCommandEvent& event );
	void SaveModel( wxCommandEvent& event );
	void OpenRecentFile( wxCommandEvent& event );
	void OnClose( wxCloseEvent& event );
	void OnExit( wxCommandEvent& event );

	void CenterView( wxCommandEvent& event );
	void SaveView( wxCommandEvent& event );
	void RestoreView( wxCommandEvent& event );
	void TakeScreenshot( wxCommandEvent& event );
	void DumpModelInfo( wxCommandEvent& event );

	void ShowMessagesWindow( wxCommandEvent& event );
	void OnCompileModel( wxCommandEvent& event );
	void OnDecompileModel( wxCommandEvent& event );
	void OnEditQC( wxCommandEvent& event );
	void OpenOptionsDialog( wxCommandEvent& event );

	void OnAbout( wxCommandEvent& event );

	void OnMessagesWindowClosed( wxCloseEvent& event );

private:
	CModelViewerApp* m_pHLMV;
	CMainPanel* m_pMainPanel;

	ui::CwxRecentFiles m_RecentFiles;

private:
	CMainWindow( const CMainWindow& ) = delete;
	CMainWindow& operator=( const CMainWindow& ) = delete;
};
}

#endif //CMAINWINDOW_H