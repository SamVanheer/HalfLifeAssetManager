#include "CMainPanel.h"

#include "CMainWindow.h"

wxBEGIN_EVENT_TABLE( CMainWindow, wxFrame )
	EVT_MENU( ID_Hello, CMainWindow::OnHello )
	EVT_MENU( wxID_EXIT, CMainWindow::OnExit )
	EVT_MENU( wxID_ABOUT, CMainWindow::OnAbout )
wxEND_EVENT_TABLE()

CMainWindow::CMainWindow()
	: wxFrame( nullptr, wxID_ANY, "Half-Life Model Viewer", wxDefaultPosition, wxSize( 600, 400 ) )
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append( ID_Hello, "&Hello...\tCtrl-H",
					  "Help string shown in status bar for this menu item" );
	menuFile->AppendSeparator();
	menuFile->Append( wxID_EXIT );
	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append( wxID_ABOUT );
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, "&File" );
	menuBar->Append( menuHelp, "&Help" );
	SetMenuBar( menuBar );
	CreateStatusBar();
	SetStatusText( "Welcome to wxWidgets!" );

	m_pMainPanel = new CMainPanel( this );

	Maximize( true );
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::OnExit( wxCommandEvent& event )
{
	Close( true );
}

void CMainWindow::OnAbout( wxCommandEvent& event )
{
	wxMessageBox( "This is a wxWidgets' Hello world sample",
				  "About Hello World", wxOK | wxICON_INFORMATION );
}

void CMainWindow::OnHello( wxCommandEvent& event )
{
	wxLogMessage( "Hello world from wxWidgets!" );
}
