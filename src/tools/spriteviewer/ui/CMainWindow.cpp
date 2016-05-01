#include <wx/filename.h>

#include "ui/wx/CwxOpenGL.h"

#include "ui/wx/shared/CMessagesWindow.h"
#include "ui/wx/utility/wxUtil.h"

#include "CSpriteViewer.h"

#include "game/entity/CSpriteEntity.h"

#include "tools/shared/Credits.h"

#include "CMainPanel.h"

#include "CMainWindow.h"

namespace sprview
{
wxBEGIN_EVENT_TABLE( CMainWindow, ui::CwxBaseFrame )
	EVT_MENU( wxID_MAINWND_LOADSPRITE, CMainWindow::LoadSprite )
	EVT_MENU( wxID_MAINWND_LOADBACKGROUND, CMainWindow::LoadBackgroundTexture )
	EVT_MENU( wxID_MAINWND_SAVESPRITE, CMainWindow::SaveSprite )
	EVT_MENU( wxID_EXIT, CMainWindow::OnExit )
	EVT_MENU( wxID_MAINWND_TAKESCREENSHOT, CMainWindow::TakeScreenshot )
	EVT_MENU( wxID_MAINWND_DUMPSPRITEINFO, CMainWindow::DumpSpriteInfo )
	EVT_MENU( wxID_MAINWND_TOGGLEMESSAGES, CMainWindow::ShowMessagesWindow )
	EVT_MENU( wxID_ABOUT, CMainWindow::OnAbout )
wxEND_EVENT_TABLE()

CMainWindow::CMainWindow( CSpriteViewer* const pSpriteViewer )
	: CwxBaseFrame( nullptr, wxID_ANY, SPRITEVIEWER_TITLE, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pSpriteViewer( pSpriteViewer )
	, m_RecentFiles( pSpriteViewer->GetSettings()->GetRecentFiles() )
{
	m_pSpriteViewer->SetMainWindow( this );

	SetIcon( m_pSpriteViewer->GetToolIcon() );

	wxMenu* menuFile = new wxMenu;

	menuFile->Append( wxID_MAINWND_LOADSPRITE, "&Load Sprite...",
					  "Load a sprite" );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_LOADBACKGROUND, "Load Background Texture..." );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_MAINWND_SAVESPRITE, "Save Sprite As...", "Save sprite to disk" );

	menuFile->AppendSeparator();

	wxMenu* pRecentFiles = new wxMenu;

	m_RecentFiles.AddMenuItems( pRecentFiles );

	//Handle the range of items.
	this->Bind( wxEVT_MENU, &CMainWindow::OpenRecentFile, this, m_RecentFiles.GetBaseID(), m_RecentFiles.GetLastID() );

	menuFile->AppendSubMenu( pRecentFiles, "Recent Files" );

	menuFile->AppendSeparator();

	menuFile->Append( wxID_EXIT );

	wxMenu* pMenuView = new wxMenu;

	pMenuView->Append( wxID_MAINWND_TAKESCREENSHOT, "Take Screenshot" );

	pMenuView->Append( wxID_MAINWND_DUMPSPRITEINFO, "Dump Sprite Info" );

	wxMenu* pMenuTools = new wxMenu;

	pMenuTools->Append( wxID_MAINWND_TOGGLEMESSAGES, "Show Messages Window", "Shows or hides the messages window", true );

	wxMenu* menuHelp = new wxMenu;

	menuHelp->Append( wxID_ABOUT );

	wxMenuBar* menuBar = new wxMenuBar;

	menuBar->Append( menuFile, "&File" );
	menuBar->Append( pMenuView, "&View" );
	menuBar->Append( pMenuTools, "&Tools" );
	menuBar->Append( menuHelp, "&Help" );

	SetMenuBar( menuBar );

	CreateStatusBar();
	SetStatusText( "" );

	m_pMainPanel = new CMainPanel( this, m_pSpriteViewer );

	Maximize( true );

	if( ui::CMessagesWindow* pWindow = m_pSpriteViewer->GetMessagesWindow() )
	{
		pWindow->Bind( wxEVT_CLOSE_WINDOW, &CMainWindow::OnMessagesWindowClosed, this );
	}
}

CMainWindow::~CMainWindow()
{
	//Clear the sprite here, while the context is still valid.
	m_pMainPanel->FreeSprite();

	m_pSpriteViewer->Exit( true );
}

void CMainWindow::RunFrame()
{
	m_pMainPanel->RunFrame();
}

bool CMainWindow::LoadSprite( const wxString& szFilename )
{
	wxFileName file( szFilename );

	file.MakeAbsolute();

	const wxString szAbsFilename = file.GetFullPath();

	if( !file.Exists() )
	{
		wxMessageBox( wxString::Format( "The file \"%s\" does not exist.", szAbsFilename ) );

		m_pSpriteViewer->GetSettings()->GetRecentFiles()->Remove( std::string( szFilename.c_str() ) );

		m_RecentFiles.Refresh();

		return false;
	}

	const bool bSuccess = m_pMainPanel->LoadSprite( szAbsFilename );

	if( bSuccess )
	{
		const wxCStrData data = szAbsFilename.c_str();
		const char* const pszAbsFilename = data.AsChar();

		this->SetTitleContent( pszAbsFilename );

		m_pSpriteViewer->GetSettings()->GetRecentFiles()->Add( pszAbsFilename );

		m_RecentFiles.Refresh();

		Message( "Loaded sprite \"%s\"\n", pszAbsFilename );
	}
	else
		this->ClearTitleContent();

	return bSuccess;
}

bool CMainWindow::PromptLoadSprite()
{
	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Sprites (*.spr)|*.spr" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return LoadSprite( dlg.GetPath() );
}

bool CMainWindow::SaveSprite( const wxString& szFilename )
{
	if( !m_pSpriteViewer->GetState()->GetEntity() )
		return false;

	auto pSprite = m_pSpriteViewer->GetState()->GetEntity()->GetSprite();

	//TODO
	const bool bSuccess = false;//sprite::SaveSprite( szFilename.c_str(), pSprite );

	if( !bSuccess )
	{
		wxMessageBox( wxString::Format( "An error occurred while saving the sprite \"%s\"", szFilename.c_str() ) );
	}

	return bSuccess;
}

bool CMainWindow::PromptSaveSprite()
{
	if( m_pSpriteViewer->GetState()->GetEntity() == nullptr )
	{
		wxMessageBox( "No sprite to save!" );
		return false;
	}

	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Half-Life Sprites (*.spr)|*.spr", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

	if( dlg.ShowModal() == wxID_CANCEL )
		return false;

	return SaveSprite( dlg.GetPath() );
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

void CMainWindow::TakeScreenshot()
{
	m_pMainPanel->TakeScreenshot();
}

void CMainWindow::DumpSpriteInfo()
{
	if( !m_pSpriteViewer->GetState()->GetEntity() )
	{
		wxMessageBox( "No sprite loaded!" );
		return;
	}

	if( m_pSpriteViewer->GetState()->DumpSpriteInfo( SPRITEVIEWER_DUMP_SPRITE_INFO_FILE ) )
	{
		//Launch the default text editor.
		if( !wx::LaunchDefaultTextEditor( SPRITEVIEWER_DUMP_SPRITE_INFO_FILE ) )
		{
			wxMessageBox( "Unable to open default text editor" );
		}
	}
	else
	{
		wxMessageBox( "An error occurred while dumping sprite info" );
	}
}

void CMainWindow::LoadSprite( wxCommandEvent& event )
{
	PromptLoadSprite();
}

void CMainWindow::LoadBackgroundTexture( wxCommandEvent& event )
{
	PromptLoadBackgroundTexture();
}

void CMainWindow::SaveSprite( wxCommandEvent& event )
{
	PromptSaveSprite();
}

void CMainWindow::OpenRecentFile( wxCommandEvent& event )
{
	wxString szFilename;
	
	if( !m_RecentFiles.OnOpenRecentFile( event, szFilename ) )
	{
		wxMessageBox( "An error occurred while trying to open a recent file" );
		return;
	}

	LoadSprite( szFilename );
}

void CMainWindow::OnExit( wxCommandEvent& event )
{
	Close( true );
}

void CMainWindow::TakeScreenshot( wxCommandEvent& event )
{
	TakeScreenshot();
}

void CMainWindow::DumpSpriteInfo( wxCommandEvent& event )
{
	DumpSpriteInfo();
}

void CMainWindow::ShowMessagesWindow( wxCommandEvent& event )
{
	m_pSpriteViewer->ShowMessagesWindow( event.IsChecked() );
}

void CMainWindow::OnAbout( wxCommandEvent& event )
{
	//This should not be modified unless necessary (e.g. to change a date, or to correct information).

	//No tab support in message boxes, so manually align it
	wxMessageBox( wxString::Format( 
					"Half-Life Sprite Viewer 2.0\n"
					"2016 Sam \"Solokiller\" Vanheer\n\n"
					"Email:    sam.vanheer@outlook.com\n\n"
					"%s", 
					tools::GetSharedCredits()
					),
					"About Half-Life Sprite Viewer", wxOK | wxICON_INFORMATION );
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
}