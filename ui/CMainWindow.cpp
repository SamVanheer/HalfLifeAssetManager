#include "CMainPanel.h"

#include "CMainWindow.h"

wxBEGIN_EVENT_TABLE( CMainWindow, wxFrame )
	EVT_MENU( wxID_MAINWND_LOADMODEL, CMainWindow::LoadModel )
	EVT_MENU( wxID_EXIT, CMainWindow::OnExit )
	EVT_MENU( wxID_ABOUT, CMainWindow::OnAbout )
wxEND_EVENT_TABLE()

CMainWindow::CMainWindow()
	: wxFrame( nullptr, wxID_ANY, "Half-Life Model Viewer", wxDefaultPosition, wxSize( 600, 400 ) )
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append( wxID_MAINWND_LOADMODEL, "&Load Model...",
					  "Load a model" );
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

void CMainWindow::LoadModel( wxCommandEvent& event )
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Models (*.mdl)|*.mdl" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pMainPanel->LoadModel( dlg.GetPath() );
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