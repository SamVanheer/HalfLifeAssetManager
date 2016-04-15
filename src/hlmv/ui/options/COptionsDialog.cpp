#include <wx/sizer.h>
#include <wx/notebook.h>

#include "CGeneralOptions.h"
#include "ui/shared/CGameConfigurations.h"

#include "hlmv/settings/CHLMVSettings.h"

#include "COptionsDialog.h"

wxBEGIN_EVENT_TABLE( COptionsDialog, wxDialog )
	EVT_BUTTON( wxID_ANY, COptionsDialog::OnButton )
wxEND_EVENT_TABLE()

COptionsDialog::COptionsDialog( wxWindow* pParent, hlmv::CHLMVSettings* const pSettings )
	: wxDialog( pParent, wxID_ANY, "Options", wxDefaultPosition, wxSize( 500, 700 ) )
	, m_pSettings( pSettings )
	, m_EditableSettings( std::make_unique<hlmv::CHLMVSettings>( *pSettings ) )
{
	m_pPages = new wxNotebook( this, wxID_ANY );

	m_pGeneral = new hlmv::CGeneralOptions( m_pPages, m_EditableSettings.get() );
	m_pGameConfigs = new ui::CGameConfigurations( m_pPages, m_EditableSettings->GetConfigManager() );

	m_pPages->AddPage( m_pGeneral, "General" );
	m_pPages->AddPage( m_pGameConfigs, "Game Configurations" );

	m_pPages->ChangeSelection( 0 );

	//Layout
	wxBoxSizer* pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( m_pPages, wxSizerFlags().Expand().Proportion( 1 ) );

	pSizer->Add( this->CreateStdDialogButtonSizer( wxOK | wxCANCEL | wxAPPLY ) );

	this->SetSizer( pSizer );

	this->CenterOnScreen();
}

COptionsDialog::~COptionsDialog()
{
}

void COptionsDialog::OnButton( wxCommandEvent& event )
{
	switch( event.GetId() )
	{
		//Save state
	case wxID_OK:
	case wxID_APPLY:
		{
			m_pGeneral->Save();
			m_pGameConfigs->Save();

			//Copy over the settings to the actual settings object.
			*m_pSettings = *m_EditableSettings;
			break;
		}

		//Do nothing
	case wxID_CANCEL:
		{
			break;
		}

	default: break;
	}

	event.Skip();
}