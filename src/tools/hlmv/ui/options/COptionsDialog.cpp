#include <wx/sizer.h>
#include <wx/notebook.h>

#include "CGeneralOptions.h"
#include "CCompilerOptions.h"
#include "ui/common/CGameConfigurationsPanel.h"

#include "ui/CModelViewerApp.h"
#include "../../settings/CHLMVSettings.h"

#include "COptionsDialog.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( COptionsDialog, wxDialog )
	EVT_BUTTON( wxID_ANY, COptionsDialog::OnButton )
wxEND_EVENT_TABLE()

COptionsDialog::COptionsDialog( wxWindow* pParent, CModelViewerApp* const pHLMV, CHLMVSettings* const pSettings )
	: wxPropertySheetDialog()
	, m_pHLMV(pHLMV)
	, m_pSettings( pSettings )
	, m_EditableSettings( std::make_unique<CHLMVSettings>( *pSettings ) )
{
	if( !wxPropertySheetDialog::Create( pParent, wxID_ANY, "Options", wxDefaultPosition, wxSize( 500, 700 ) ) )
		return;

	CreateButtons( wxOK | wxCANCEL | wxAPPLY );

	auto pBook = GetBookCtrl();

	//TODO: refactor to use a base class and list
	m_pGeneral = new CGeneralOptions( pBook, m_EditableSettings.get() );
	m_pCompiler = new CCompilerOptions( pBook, m_EditableSettings.get() );
	m_pGameConfigs = new ui::CGameConfigurationsPanel( pBook, m_EditableSettings->GetConfigManager() );

	pBook->AddPage( m_pGeneral, "General" );
	pBook->AddPage( m_pCompiler, "Compiler" );
	pBook->AddPage( m_pGameConfigs, "Game Configurations" );

	pBook->ChangeSelection( 0 );

	pBook->SetMinSize( wxSize( 600, 420 ) );

	//Layout
	LayoutDialog();
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
			m_pCompiler->Save();
			m_pGameConfigs->Save();

			const bool updateTimer = m_pSettings->UseTimerForFrame() != m_EditableSettings->UseTimerForFrame();

			//Copy over the settings to the actual settings object.
			*m_pSettings = *m_EditableSettings;

			if (updateTimer)
			{
				m_pHLMV->ResetTickImplementation();
			}
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
}