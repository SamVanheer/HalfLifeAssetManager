#include <wx/gbsizer.h>

#include "settings/CGameConfigManager.h"
#include "settings/CGameConfig.h"

#include "CGameConfigurationsPanel.h"

#include "CEditGameConfigsDialog.h"

namespace ui
{
wxBEGIN_EVENT_TABLE( CEditGameConfigsDialog, wxDialog )
	EVT_BUTTON( wxID_SHARED_EDITGAMECONFIGS_ADD, CEditGameConfigsDialog::AddConfig )
	EVT_BUTTON( wxID_SHARED_EDITGAMECONFIGS_EDIT, CEditGameConfigsDialog::EditConfig )
	EVT_BUTTON( wxID_SHARED_EDITGAMECONFIGS_REMOVE, CEditGameConfigsDialog::RemoveConfig )
wxEND_EVENT_TABLE()

CEditGameConfigsDialog::CEditGameConfigsDialog( wxWindow* pParent, std::shared_ptr<settings::CGameConfigManager> manager, CGameConfigurationsPanel* const pGameConfigsPanel )
	: wxDialog( pParent, wxID_ANY, "Edit Game Configurations", wxDefaultPosition, wxSize( 400, 250 ) )
	, m_Manager( manager )
	, m_pGameConfigsPanel( pGameConfigsPanel )
{
	wxASSERT( manager != nullptr );

	m_pConfigs = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( 200, 200 ) );

	wxButton* pAdd = new wxButton( this, wxID_SHARED_EDITGAMECONFIGS_ADD, "Add" );
	wxButton* pEdit = new wxButton( this, wxID_SHARED_EDITGAMECONFIGS_EDIT, "Edit" );
	wxButton* pRemove = new wxButton( this, wxID_SHARED_EDITGAMECONFIGS_REMOVE, "Remove" );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Game Configurations:" ), wxGBPosition( 0, 0 ), wxGBSpan( 1, 6 ) );
	pSizer->Add( m_pConfigs, wxGBPosition( 1, 0 ), wxGBSpan( 3, 4 ) );

	pSizer->Add( pAdd, wxGBPosition( 1, 4 ), wxGBSpan( 1, 2 ) );
	pSizer->Add( pEdit, wxGBPosition( 2, 4 ), wxGBSpan( 1, 2 ) );
	pSizer->Add( pRemove, wxGBPosition( 3, 4 ), wxGBSpan( 1, 2 ) );

	pSizer->Add( this->CreateStdDialogButtonSizer( wxOK ), wxGBPosition( 4, 0 ), wxGBSpan( 1, 6 ) );

	for( size_t uiIndex = 0; uiIndex < 6; ++uiIndex )
	{
		pSizer->AddGrowableCol( uiIndex );
	}

	this->SetSizer( pSizer );

	this->Fit();

	Initialize();
}

CEditGameConfigsDialog::~CEditGameConfigsDialog()
{
}

void CEditGameConfigsDialog::Initialize()
{
	m_pConfigs->Clear();

	wxArrayString configs;

	for( const auto& config : m_Manager->GetConfigs() )
	{
		configs.push_back( config->GetName() );
	}

	m_pConfigs->Append( configs );
}

bool CEditGameConfigsDialog::AddConfig( const wxString& szName )
{
	if( szName.IsEmpty() )
		return false;

	auto config = std::make_shared<settings::CGameConfig>( szName.c_str() );

	if( !m_Manager->AddConfig( config ) )
		return false;

	m_pConfigs->Append( szName );

	m_pGameConfigsPanel->ConfigAdded( szName );

	return true;
}

void CEditGameConfigsDialog::AddConfig( wxCommandEvent& event )
{
	wxTextEntryDialog dlg( this, "Enter a unique name for the configuration", "Add new configuration" );

	do
	{
		if( dlg.ShowModal() == wxID_CANCEL )
			return;

		const wxString szValue = dlg.GetValue();

		if( !szValue.IsEmpty() && !m_Manager->HasConfig( szValue.c_str() ) )
		{
			if( AddConfig( szValue ) )
			{
				return;
			}
			else
			{
				wxMessageBox( wxString::Format( "Error adding configuration \"%s\"", szValue ) );
			}
		}
	}
	while( true );
}

void CEditGameConfigsDialog::EditConfig( wxCommandEvent& event )
{
	const int iIndex = m_pConfigs->GetSelection();

	if( iIndex == wxNOT_FOUND )
		return;

	const wxString szName = m_pConfigs->GetString( iIndex );

	auto config = m_Manager->GetConfig( szName.c_str() );

	if( !config )
		return;

	wxTextEntryDialog dlg( this, "Enter a unique name for the configuration", "Edit configuration name" );

	dlg.SetValue( szName );

	do
	{
		if( dlg.ShowModal() == wxID_CANCEL )
			return;

		const wxString szValue = dlg.GetValue();

		if( !szValue.IsEmpty() )
		{
			if( m_Manager->RenameConfig( szName.c_str(), szValue.c_str() ) )
			{
				m_pConfigs->SetString( iIndex, szValue );
				m_pGameConfigsPanel->ConfigRenamed( szName, szValue );
				return;
			}
		}
	}
	while( true );
}

void CEditGameConfigsDialog::RemoveConfig( wxCommandEvent& event )
{
	const int iIndex = m_pConfigs->GetSelection();

	if( iIndex == wxNOT_FOUND )
		return;

	const wxString szName = m_pConfigs->GetString( iIndex );

	auto config = m_Manager->GetConfig( szName.c_str() );

	if( !config )
		return;

	if( !m_Manager->RemoveConfig( config ) )
		return;

	m_pConfigs->Delete( iIndex );

	m_pGameConfigsPanel->ConfigRemoved( szName );
}
}