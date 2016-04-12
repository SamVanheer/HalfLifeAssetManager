#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "wxHLMV.h"

#include "ui/utility/CTimer.h"

class CTimer;
class CMainPanel;
class CHLMVSettings;

class CMainWindow final : public wxFrame, public ITimerListener
{
public:
	CMainWindow( CHLMVSettings* const pSettings );
	~CMainWindow();

	void OnTimer( CTimer& timer ) override final;

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

private:
	void LoadModel( wxCommandEvent& event );
	void LoadBackgroundTexture( wxCommandEvent& event );
	void LoadGroundTexture( wxCommandEvent& event );
	void UnloadGroundTexture( wxCommandEvent& event );
	void SaveModel( wxCommandEvent& event );

	void OnExit( wxCommandEvent& event );
	void OnAbout( wxCommandEvent& event );

	wxDECLARE_EVENT_TABLE();

private:
	CMainPanel* m_pMainPanel;
	CHLMVSettings* m_pSettings;

private:
	CMainWindow( const CMainWindow& ) = delete;
	CMainWindow& operator=( const CMainWindow& ) = delete;
};

#endif //CMAINWINDOW_H