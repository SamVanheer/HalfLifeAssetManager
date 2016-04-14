#include <wx/filename.h>

#include "hlmv/ui/CModelViewerApp.h"

#include "graphics/OpenGL.h"

#include "hlmv/CHLMVState.h"

#include "filesystem/CFileSystem.h"

#include "utility/CString.h"

#include "ui/shared/CMessagesWindow.h"
#include "options/COptionsDialog.h"

#include "CMainPanel.h"

#include "CMainWindow.h"

wxBEGIN_EVENT_TABLE( CMainWindow, wxFrame )
	EVT_MENU( wxID_MAINWND_LOADMODEL, CMainWindow::LoadModel )
	EVT_MENU( wxID_MAINWND_LOADBACKGROUND, CMainWindow::LoadBackgroundTexture )
	EVT_MENU( wxID_MAINWND_LOADGROUND, CMainWindow::LoadGroundTexture )
	EVT_MENU( wxID_MAINWND_UNLOADGROUND, CMainWindow::UnloadGroundTexture )
	EVT_MENU( wxID_MAINWND_SAVEMODEL, CMainWindow::SaveModel )
	EVT_MENU( wxID_MAINWND_RECENTFILE1, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE2, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE3, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE4, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_EXIT, CMainWindow::OnExit )
	EVT_MENU( wxID_MAINWND_TOGGLEMESSAGES, CMainWindow::ShowMessagesWindow )
	EVT_MENU( wxID_MAINWND_OPTIONS, CMainWindow::OpenOptionsDialog )
	EVT_MENU( wxID_ABOUT, CMainWindow::OnAbout )
wxEND_EVENT_TABLE()

CMainWindow::CMainWindow( hlmv::CHLMVState* const pSettings )
	: wxFrame( nullptr, wxID_ANY, HLMV_TITLE, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pSettings( pSettings )
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

	wxMenu* pRecentFiles = new wxMenu;

	for( size_t uiIndex = 0; uiIndex < hlmv::CHLMVState::MAX_RECENT_FILES; ++uiIndex )
	{
		wxMenuItem* pItem = m_RecentFiles[ uiIndex ] = pRecentFiles->Append( wxID_MAINWND_RECENTFILE1 + static_cast<int>( uiIndex ), "(empty)" );
	}

	RefreshRecentFiles();

	menuFile->AppendSubMenu( pRecentFiles, "Recent Files" );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_EXIT );

	wxMenu* pMenuTools = new wxMenu;

	pMenuTools->Append( wxID_MAINWND_TOGGLEMESSAGES, "Show Messages Window", "Shows or hides the messages window", true );

	pMenuTools->AppendSeparator();

	pMenuTools->Append( wxID_MAINWND_OPTIONS, "Options" );

	wxMenu* menuHelp = new wxMenu;

	menuHelp->Append( wxID_ABOUT );

	wxMenuBar* menuBar = new wxMenuBar;

	menuBar->Append( menuFile, "&File" );
	menuBar->Append( pMenuTools, "&Tools" );
	menuBar->Append( menuHelp, "&Help" );

	SetMenuBar( menuBar );

	CreateStatusBar();
	SetStatusText( "" );

	m_pMainPanel = new CMainPanel( this, m_pSettings );

	Maximize( true );

	if( CMessagesWindow* pWindow = wxGetApp().GetMessagesWindow() )
	{
		pWindow->Bind( wxEVT_CLOSE_WINDOW, &CMainWindow::OnMessagesWindowClosed, this );
	}

	//LoadModel( "barney.mdl" );
}

CMainWindow::~CMainWindow()
{
	//Call this first so nothing tries to access the settings object.
	DestroyChildren();

	m_pSettings->ClearStudioModel();

	wxGetApp().ExitApp( true );
}

void CMainWindow::OnTimer( CTimer& timer )
{
	m_pMainPanel->OnTimer( timer );
}

bool CMainWindow::LoadModel( const wxString& szFilename )
{
	wxFileName file( szFilename );

	file.MakeAbsolute();

	const wxString szAbsFilename = file.GetFullPath();

	const bool bSuccess = m_pMainPanel->LoadModel( szAbsFilename );

	if( bSuccess )
	{
		const wxCStrData data = szAbsFilename.c_str();
		const char* const pszAbsFilename = data.AsChar();

		this->SetTitle( wxString::Format( "%s - %s", HLMV_TITLE, pszAbsFilename ) );

		m_pSettings->recentFiles->Add( pszAbsFilename );

		//TODO: if the file doesn't exist, remove the entry.
		RefreshRecentFiles();

		Message( "Loaded model \"%s\"\n", pszAbsFilename );
	}
	else
		this->SetTitle( HLMV_TITLE );

	return bSuccess;
}

bool CMainWindow::PromptLoadModel()
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Models (*.mdl)|*.mdl" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return LoadModel( dlg.GetPath() );
}

bool CMainWindow::SaveModel( const wxString& szFilename )
{
	if( !m_pSettings->GetStudioModel() )
		return false;

	const bool bSuccess = m_pSettings->GetStudioModel()->SaveModel( szFilename.char_str( wxMBConvUTF8() ) );

	if( !bSuccess )
	{
		wxMessageBox( wxString::Format( "An error occurred while saving the model \"%s\"", szFilename.c_str() ) );
	}

	return bSuccess;
}

bool CMainWindow::PromptSaveModel()
{
	if( m_pSettings->GetStudioModel() == nullptr )
	{
		wxMessageBox( "No model to save!" );
		return false;
	}

	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Models (*.mdl)|*.mdl", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return SaveModel( dlg.GetPath() );
}

bool CMainWindow::LoadBackgroundTexture( const wxString& szFilename )
{
	return m_pMainPanel->LoadBackgroundTexture( szFilename );
}

bool CMainWindow::PromptLoadBackgroundTexture()
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "*.*" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return LoadBackgroundTexture( dlg.GetPath() );
}

void CMainWindow::UnloadBackgroundTexture()
{
	m_pMainPanel->UnloadBackgroundTexture();
}

bool CMainWindow::LoadGroundTexture( const wxString& szFilename )
{
	return m_pMainPanel->LoadGroundTexture( szFilename );
}

bool CMainWindow::PromptLoadGroundTexture()
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "*.*" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return LoadGroundTexture( dlg.GetPath() );
}

void CMainWindow::UnloadGroundTexture()
{
	m_pMainPanel->UnloadGroundTexture();
}

void CMainWindow::RefreshRecentFiles()
{
	const auto& recentFiles = m_pSettings->recentFiles->GetFiles();

	auto it = recentFiles.begin();
	auto end = recentFiles.end();

	for( size_t uiIndex = 0; uiIndex < hlmv::CHLMVState::MAX_RECENT_FILES; ++uiIndex )
	{
		wxMenuItem* const pItem = m_RecentFiles[ uiIndex ];

		if( it != end )
		{
			pItem->Enable( true );
			pItem->SetItemLabel( it->c_str() );

			++it;
		}
		else
		{
			pItem->Enable( false );
			pItem->SetItemLabel( "(empty)" );
		}
	}
}

void CMainWindow::LoadModel( wxCommandEvent& event )
{
	PromptLoadModel();
}

void CMainWindow::LoadBackgroundTexture( wxCommandEvent& event )
{
	PromptLoadBackgroundTexture();
}

void CMainWindow::LoadGroundTexture( wxCommandEvent& event )
{
	PromptLoadGroundTexture();
}

void CMainWindow::UnloadGroundTexture( wxCommandEvent& event )
{
	UnloadGroundTexture();
}

void CMainWindow::SaveModel( wxCommandEvent& event )
{
	PromptSaveModel();
}

void CMainWindow::OpenRecentFile( wxCommandEvent& event )
{
	if( event.GetId() < wxID_MAINWND_RECENTFILE1 || event.GetId() > wxID_MAINWND_RECENTFILE4 )
	{
		wxMessageBox( wxString::Format( "Invalid ID specified for recent files list (%d)", event.GetId() ) );
		return;
	}

	const size_t fileId = static_cast<size_t>( event.GetId() - wxID_MAINWND_RECENTFILE1 );

	const wxString szFilename = m_pSettings->recentFiles->Get( fileId );

	LoadModel( szFilename );
}

void CMainWindow::OnExit( wxCommandEvent& event )
{
	Close( true );
}

void CMainWindow::ShowMessagesWindow( wxCommandEvent& event )
{
	wxGetApp().ShowMessagesWindow( event.IsChecked() );
}

void CMainWindow::OpenOptionsDialog( wxCommandEvent& event )
{
	COptionsDialog dlg( this, m_pSettings );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;
}

void CMainWindow::OnAbout( wxCommandEvent& event )
{
	//TODO: add info about 1.25, Jed's
	wxMessageBox( "Half-Life Model Viewer 1.40 By Sam \"Solokiller\" Vanheer",
				  "About Half-Life Model Viewer", wxOK | wxICON_INFORMATION );
}

void CMainWindow::OnMessagesWindowClosed( wxCloseEvent& event )
{
	//Whenever the messages window is closed by clicking the close button, uncheck this.
	wxMenuItem* pItem = GetMenuBar()->FindItem( wxID_MAINWND_TOGGLEMESSAGES );

	if( pItem )
	{
		pItem->Check( false );
	}

	event.Skip();
}