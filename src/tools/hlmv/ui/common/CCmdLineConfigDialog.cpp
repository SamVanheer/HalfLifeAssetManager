#include <algorithm>

#include <wx/editlbox.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "CEditCmdLineConfigDialog.h"

#include "CCmdLineConfigDialog.h"

namespace ui
{
enum
{
	wxID_FIRST_ID = wxID_HIGHEST + 1,

	wxID_CONFIG = wxID_FIRST_ID,
	wxID_EDIT_CONFIG,
	wxID_ADD_CONFIG,
	wxID_REMOVE_CONFIG,
};

wxBEGIN_EVENT_TABLE( CCmdLineConfigDialog, wxDialog )
	EVT_CHOICE( wxID_CONFIG, CCmdLineConfigDialog::OnConfigChanged )
	EVT_BUTTON( wxID_EDIT_CONFIG, CCmdLineConfigDialog::OnEditConfig )
	EVT_BUTTON( wxID_ADD_CONFIG, CCmdLineConfigDialog::OnAddConfig )
	EVT_BUTTON( wxID_REMOVE_CONFIG, CCmdLineConfigDialog::OnRemoveConfig )
wxEND_EVENT_TABLE()

CCmdLineConfigDialog::CCmdLineConfigDialog( wxWindow *parent, wxWindowID id,
											const wxString& title,
											const wxString& szDefaultOutputFileDir,
											std::shared_ptr<settings::CCmdLineConfigManager> manager,
											const wxPoint& pos,
											const wxSize& size,
											long style,
											const wxString& name )
	: wxDialog( parent, id, title, pos, size, style, name )
	, m_szDefaultOutputFileDir( szDefaultOutputFileDir )
	, m_Manager( manager )
	, m_MutableManager( std::make_shared<settings::CCmdLineConfigManager>( *manager ) )
{
	m_pConfigs = new wxChoice( this, wxID_CONFIG, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );

	m_pEditConfig = new wxButton( this, wxID_EDIT_CONFIG, "Edit" );

	auto pAddConfig = new wxButton( this, wxID_ADD_CONFIG, "Add" );

	m_pRemoveConfig = new wxButton( this, wxID_REMOVE_CONFIG, "Remove" );

	//Layout
	auto pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Configuration" ), wxSizerFlags().Expand().DoubleBorder() );

	{
		auto pConfigsSizer = new wxBoxSizer( wxHORIZONTAL );

		pConfigsSizer->Add( m_pConfigs, wxSizerFlags().Expand() );

		pConfigsSizer->Add( m_pEditConfig, wxSizerFlags().Expand() );

		pConfigsSizer->Add( pAddConfig, wxSizerFlags().Expand() );

		pConfigsSizer->Add( m_pRemoveConfig, wxSizerFlags().Expand() );

		pSizer->Add( pConfigsSizer, wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT ) );
	}

	pSizer->Add( new wxStaticLine( this ), wxSizerFlags().Expand().DoubleBorder() );

	pSizer->Add( this->CreateButtonSizer( wxOK | wxCANCEL ), wxSizerFlags().Expand().DoubleBorder( wxLEFT | wxRIGHT | wxDOWN ) );

	this->SetSizer( pSizer );

	this->Fit();

	this->CenterOnScreen();

	Initialize();
}

CCmdLineConfigDialog::~CCmdLineConfigDialog()
{
}

void CCmdLineConfigDialog::SetCopySupportEnabled( const bool bEnabled )
{
	m_bCopyOutputFilesEnabled = bEnabled;
}

void CCmdLineConfigDialog::SetConfig( const char* const pszName )
{
	wxASSERT( pszName );

	m_pConfigs->SetSelection( m_pConfigs->FindString( pszName ) );
}

void CCmdLineConfigDialog::SetConfig( int iIndex )
{
	if( iIndex < 0 || static_cast<size_t>( iIndex ) >= m_pConfigs->GetCount() )
		iIndex = 0;

	SetConfig( m_pConfigs->GetString( iIndex ) );
}

void CCmdLineConfigDialog::Initialize()
{
	m_pConfigs->Clear();

	wxArrayString list;

	for( const auto& config : m_MutableManager->GetConfigs() )
	{
		list.push_back( config->GetName() );
	}

	m_pConfigs->Append( list );

	m_pConfigs->Enable( !list.IsEmpty() );

	m_pEditConfig->Enable( !list.IsEmpty() );
	m_pRemoveConfig->Enable( !list.IsEmpty() );

	if( auto config = m_MutableManager->GetActiveConfig() )
	{
		SetConfig( config->GetName().c_str() );
	}
	else
	{
		SetConfig( 0 );
	}
}

void CCmdLineConfigDialog::Save()
{
	std::shared_ptr<settings::CCmdLineConfig> activeConfig;

	if( m_pConfigs->GetSelection() != wxNOT_FOUND )
	{
		activeConfig = m_MutableManager->GetConfig( m_pConfigs->GetStringSelection() );
	}

	m_MutableManager->SetActiveConfig( activeConfig );

	*m_Manager = *m_MutableManager;
}

void CCmdLineConfigDialog::OnConfigChanged( wxCommandEvent& event )
{
	SetConfig( m_pConfigs->GetSelection() );
}

void CCmdLineConfigDialog::OnEditConfig( wxCommandEvent& event )
{
	if( m_pConfigs->GetSelection() == wxNOT_FOUND )
	{
		return;
	}

	auto activeConfig = m_MutableManager->GetConfig( m_pConfigs->GetStringSelection() );

	ui::CEditCmdLineConfigDialog editDlg( this, wxID_ANY, m_MutableManager, activeConfig );

	editDlg.SetCopySupportEnabled( m_bCopyOutputFilesEnabled );

	editDlg.ShowModal();
}

void CCmdLineConfigDialog::OnAddConfig( wxCommandEvent& event )
{
	wxTextEntryDialog dlg( this, "Enter a unique name for the configuration", "Add new configuration" );

	bool bValid = false;

	wxString szName;

	std::shared_ptr<settings::CCmdLineConfig> config;

	do
	{
		if( dlg.ShowModal() == wxID_CANCEL )
			return;

		szName = dlg.GetValue();

		szName.Trim( true );
		szName.Trim( false );

		//TODO: use a validator to prevent leading and trailing whitespace from being entered.
		dlg.SetValue( szName );

		if( !szName.IsEmpty() && !m_MutableManager->HasConfig( szName ) )
		{
			config = std::make_shared<settings::CCmdLineConfig>( szName.c_str().AsChar() );

			if( m_MutableManager->AddConfig( config ) )
			{
				bValid = true;
			}
			else
			{
				wxMessageBox( wxString::Format( "Error adding configuration \"%s\"", szName ) );
			}
		}
	}
	while( !bValid );

	config->SetOutputFileDirectory( m_szDefaultOutputFileDir.c_str().AsChar() );

	m_pConfigs->Append( szName );

	m_pConfigs->Enable( true );
	m_pEditConfig->Enable( true );
	m_pRemoveConfig->Enable( true );

	//Change to new config.
	SetConfig( m_pConfigs->GetCount() - 1 );
}

void CCmdLineConfigDialog::OnRemoveConfig( wxCommandEvent& event )
{
	const int iCurrent = m_pConfigs->GetSelection();

	if( iCurrent == wxNOT_FOUND )
		return;

	m_MutableManager->RemoveConfig( m_pConfigs->GetStringSelection() );

	m_pConfigs->Delete( iCurrent );

	if( m_pConfigs->IsEmpty() )
	{
		m_pConfigs->Enable( false );
		m_pEditConfig->Enable( false );
		m_pRemoveConfig->Enable( false );
	}

	SetConfig( m_pConfigs->GetSelection() );
}
}