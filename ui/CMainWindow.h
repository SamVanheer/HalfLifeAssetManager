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
	void OnHello( wxCommandEvent& event );
	void OnExit( wxCommandEvent& event );
	void OnAbout( wxCommandEvent& event );

	wxDECLARE_EVENT_TABLE();

private:
	CMainPanel* m_pMainPanel;

private:
	CMainWindow( const CMainWindow& ) = delete;
	CMainWindow& operator=( const CMainWindow& ) = delete;
};

enum
{
	ID_Hello = 1
};

#endif //CMAINWINDOW_H