#include "model/utility/OpenGL.h"

#include "CMainPanel.h"

#include "CMainWindow.h"

wxBEGIN_EVENT_TABLE( CMainWindow, wxFrame )
	EVT_MENU( wxID_MAINWND_LOADMODEL, CMainWindow::LoadModel )
	EVT_MENU( wxID_MAINWND_LOADBACKGROUND, CMainWindow::LoadBackgroundTexture )
	EVT_MENU( wxID_MAINWND_LOADGROUND, CMainWindow::LoadGroundTexture )
	EVT_MENU( wxID_MAINWND_UNLOADGROUND, CMainWindow::UnloadGroundTexture )
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

	menuFile->Append( wxID_MAINWND_LOADBACKGROUND, "Load Background Texture..." );

	menuFile->Append( wxID_MAINWND_LOADGROUND, "Load Ground Texture..." );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_UNLOADGROUND, "Unload Ground Texture" );

	menuFile->AppendSeparator();

	//TODO: add recent files list
	//menuFile->AppendSubMenu( ... );

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

void CMainWindow::LoadBackgroundTexture( wxCommandEvent& event )
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "*.*" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pMainPanel->LoadBackgroundTexture( dlg.GetPath() );
}

void CMainWindow::LoadGroundTexture( wxCommandEvent& event )
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "*.*" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pMainPanel->LoadGroundTexture( dlg.GetPath() );
}

void CMainWindow::UnloadGroundTexture( wxCommandEvent& event )
{
	m_pMainPanel->UnloadGroundTexture();
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