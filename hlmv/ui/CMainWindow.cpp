#include <wx/filename.h>

#include "model/utility/OpenGL.h"

#include "CMainPanel.h"

#include "CMainWindow.h"

#define HLMV_TITLE "Half-Life Model Viewer"

wxBEGIN_EVENT_TABLE( CMainWindow, wxFrame )
	EVT_MENU( wxID_MAINWND_LOADMODEL, CMainWindow::LoadModel )
	EVT_MENU( wxID_MAINWND_LOADBACKGROUND, CMainWindow::LoadBackgroundTexture )
	EVT_MENU( wxID_MAINWND_LOADGROUND, CMainWindow::LoadGroundTexture )
	EVT_MENU( wxID_MAINWND_UNLOADGROUND, CMainWindow::UnloadGroundTexture )
	EVT_MENU( wxID_MAINWND_SAVEMODEL, CMainWindow::SaveModel )
	EVT_MENU( wxID_EXIT, CMainWindow::OnExit )
	EVT_MENU( wxID_ABOUT, CMainWindow::OnAbout )
wxEND_EVENT_TABLE()

CMainWindow::CMainWindow()
	: wxFrame( nullptr, wxID_ANY, "Half-Life Model Viewer", wxDefaultPosition, wxSize( 600, 400 ) )
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append( wxID_MAINWND_LOADMODEL, "&Load Model...",
					  "Load a model" );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_LOADBACKGROUND, "Load Background Texture..." );

	menuFile->Append( wxID_MAINWND_LOADGROUND, "Load Ground Texture..." );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_UNLOADGROUND, "Unload Ground Texture" );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_SAVEMODEL, "Save Model As...", "Save model to disk" );

	menuFile->AppendSeparator();

	//TODO: add recent files list
	//menuFile->AppendSubMenu( ... );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_EXIT );

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append( wxID_ABOUT );

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append( menuFile, "&File" );
	menuBar->Append( menuHelp, "&Help" );
	SetMenuBar( menuBar );

	CreateStatusBar();
	SetStatusText( "" );

	m_pMainPanel = new CMainPanel( this );

	Maximize( true );

	LoadModel( "barney.mdl" );
}

CMainWindow::~CMainWindow()
{
}

bool CMainWindow::LoadModel( const wxString& szFilename )
{
	wxFileName file( szFilename );

	file.MakeAbsolute();

	const wxString szAbsFilename = file.GetFullPath();

	const bool bSuccess = m_pMainPanel->LoadModel( szAbsFilename );

	if( bSuccess )
	{
		this->SetTitle( wxString::Format( "%s - %s", HLMV_TITLE, szAbsFilename.c_str() ) );
	}
	else
		this->SetTitle( HLMV_TITLE );

	return bSuccess;
}

void CMainWindow::LoadModel( wxCommandEvent& event )
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Models (*.mdl)|*.mdl" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	if( m_pMainPanel->LoadModel( dlg.GetPath() ) )
	{
		this->SetTitle( wxString::Format( "%s - %s", HLMV_TITLE, dlg.GetPath().c_str() ) );
	}
	else
		this->SetTitle( HLMV_TITLE );
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

void CMainWindow::SaveModel( wxCommandEvent& event )
{
	if( g_studioModel.getStudioHeader() == nullptr )
	{
		wxMessageBox( "No model to save!" );
		return;
	}

	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Models (*.mdl)|*.mdl", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szFilename = dlg.GetPath();

	if( !g_studioModel.SaveModel( szFilename.char_str( wxMBConvUTF8() ) ) )
	{
		wxMessageBox( wxString::Format( "An error occurred while saving the model \"%s\"", szFilename.c_str() ) );
	}
}

void CMainWindow::OnExit( wxCommandEvent& event )
{
	Close( true );
}

void CMainWindow::OnAbout( wxCommandEvent& event )
{
	wxMessageBox( "Half-Life Model Viewer 1.40 By Sam \"Solokiller\" Vanheer",
				  "About Half-Life Model Viewer", wxOK | wxICON_INFORMATION );
}