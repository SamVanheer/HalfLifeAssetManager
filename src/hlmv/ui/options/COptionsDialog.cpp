#include <wx/sizer.h>
#include <wx/notebook.h>

#include "ui/shared/CGameConfigurations.h"

#include "hlmv/CHLMVState.h"

#include "COptionsDialog.h"

wxBEGIN_EVENT_TABLE( COptionsDialog, wxDialog )
	EVT_BUTTON( wxID_ANY, COptionsDialog::OnButton )
wxEND_EVENT_TABLE()

COptionsDialog::COptionsDialog( wxWindow* pParent, hlmv::CHLMVState* const pSettings )
	: wxDialog( pParent, wxID_ANY, "Options", wxDefaultPosition, wxSize( 500, 700 ) )
	, m_pSettings( pSettings )
{
	m_pPages = new wxNotebook( this, wxID_ANY );

	//TODO: use a copy of the manager
	m_pGameConfigs = new CGameConfigurations( m_pPages, m_pSettings->configManager );

	m_pPages->AddPage( m_pGameConfigs, "Game Configurations", true );

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
			//Copy over the settings to the actual settings object.
			//*m_pSettings = *m_pConfSettings;

			m_pGameConfigs->Save();
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