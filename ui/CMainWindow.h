#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "wxInclude.h"

class CMainPanel;

class CMainWindow final : public wxFrame
{
public:
	CMainWindow();
	~CMainWindow();

private:
	void LoadModel( wxCommandEvent& event );
	void LoadBackgroundTexture( wxCommandEvent& event );
	void LoadGroundTexture( wxCommandEvent& event );
	void UnloadGroundTexture( wxCommandEvent& event );

	void OnExit( wxCommandEvent& event );
	void OnAbout( wxCommandEvent& event );

	wxDECLARE_EVENT_TABLE();

private:
	CMainPanel* m_pMainPanel;

private:
	CMainWindow( const CMainWindow& ) = delete;
	CMainWindow& operator=( const CMainWindow& ) = delete;
};

#endif //CMAINWINDOW_H