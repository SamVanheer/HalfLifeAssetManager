#include <wx/filename.h>
#include <wx/mimetype.h>

#include "ui/wx/CwxOpenGL.h"

#include "ui/wx/shared/CMessagesWindow.h"

#include "CSpriteViewer.h"

#include "game/entity/CSpriteEntity.h"

#include "tools/shared/Credits.h"

#include "CMainPanel.h"

#include "CMainWindow.h"

namespace sprview
{
wxBEGIN_EVENT_TABLE( CMainWindow, wxFrame )
	EVT_MENU( wxID_MAINWND_LOADSPRITE, CMainWindow::LoadSprite )
	EVT_MENU( wxID_MAINWND_LOADBACKGROUND, CMainWindow::LoadBackgroundTexture )
	EVT_MENU( wxID_MAINWND_SAVESPRITE, CMainWindow::SaveSprite )
	EVT_MENU( wxID_MAINWND_RECENTFILE1, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE2, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE3, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_MAINWND_RECENTFILE4, CMainWindow::OpenRecentFile )
	EVT_MENU( wxID_EXIT, CMainWindow::OnExit )
	EVT_MENU( wxID_MAINWND_TAKESCREENSHOT, CMainWindow::TakeScreenshot )
	EVT_MENU( wxID_MAINWND_DUMPSPRITEINFO, CMainWindow::DumpSpriteInfo )
	EVT_MENU( wxID_MAINWND_TOGGLEMESSAGES, CMainWindow::ShowMessagesWindow )
	EVT_MENU( wxID_ABOUT, CMainWindow::OnAbout )
wxEND_EVENT_TABLE()

CMainWindow::CMainWindow( CSpriteViewer* const pSpriteViewer )
	: wxFrame( nullptr, wxID_ANY, SPRITEVIEWER_TITLE, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pSpriteViewer( pSpriteViewer )
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

	for( size_t uiIndex = 0; uiIndex < CSpriteViewerSettings::MAX_RECENT_FILES; ++uiIndex )
	{
		wxMenuItem* pItem = m_RecentFiles[ uiIndex ] = pRecentFiles->Append( wxID_MAINWND_RECENTFILE1 + static_cast<int>( uiIndex ), "(empty)" );
	}

	RefreshRecentFiles();

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

		RefreshRecentFiles();

		return false;
	}

	const bool bSuccess = m_pMainPanel->LoadSprite( szAbsFilename );

	if( bSuccess )
	{
		const wxCStrData data = szAbsFilename.c_str();
		const char* const pszAbsFilename = data.AsChar();

		this->SetTitle( wxString::Format( "%s - %s", SPRITEVIEWER_TITLE, pszAbsFilename ) );

		m_pSpriteViewer->GetSettings()->GetRecentFiles()->Add( pszAbsFilename );

		RefreshRecentFiles();

		Message( "Loaded sprite \"%s\"\n", pszAbsFilename );
	}
	else
		this->SetTitle( SPRITEVIEWER_TITLE );

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
		wxFileType* pFileType = wxTheMimeTypesManager->GetFileTypeFromExtension( "txt" );

		bool bSuccess = false;

		if( pFileType )
		{
			wxString szOpenCommand;

			if( pFileType->GetOpenCommand( &szOpenCommand, wxFileType::MessageParameters( SPRITEVIEWER_DUMP_SPRITE_INFO_FILE ) ) )
			{
				bSuccess = wxExecute( szOpenCommand, wxEXEC_ASYNC ) != 0;
			}

			delete pFileType;
		}

		if( !bSuccess )
		{
			wxMessageBox( "Unable to open default text editor" );
		}
	}
	else
	{
		wxMessageBox( "An error occurred while dumping model info" );
	}
}

void CMainWindow::RefreshRecentFiles()
{
	const auto& recentFiles = m_pSpriteViewer->GetSettings()->GetRecentFiles()->GetFiles();

	auto it = recentFiles.begin();
	auto end = recentFiles.end();

	for( size_t uiIndex = 0; uiIndex < CSpriteViewerSettings::MAX_RECENT_FILES; ++uiIndex )
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
	if( event.GetId() < wxID_MAINWND_RECENTFILE1 || event.GetId() > wxID_MAINWND_RECENTFILE4 )
	{
		wxMessageBox( wxString::Format( "Invalid ID specified for recent files list (%d)", event.GetId() ) );
		return;
	}

	const size_t fileId = static_cast<size_t>( event.GetId() - wxID_MAINWND_RECENTFILE1 );

	const wxString szFilename = m_pSpriteViewer->GetSettings()->GetRecentFiles()->Get( fileId );

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
}