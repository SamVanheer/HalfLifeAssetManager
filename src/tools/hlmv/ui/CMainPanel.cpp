#include <memory>

#include <wx/notebook.h>
#include <wx/gbsizer.h>

#include "C3DView.h"

#include "controlpanels/CModelDisplayPanel.h"
#include "controlpanels/CBodyPartsPanel.h"
#include "controlpanels/CTexturesPanel.h"
#include "controlpanels/CSequencesPanel.h"
#include "controlpanels/CFullscreenPanel.h"

#include "game/studiomodel/StudioModel.h"

#include "game/studiomodel/CStudioModel.h"
#include "game/entity/CStudioModelEntity.h"
#include "game/entity/CBaseEntityList.h"

#include "CHLMV.h"
#include "../CHLMVState.h"

#include "CMainPanel.h"

namespace hlmv
{
static const wxString VIEWORIGINS[] = 
{
	"Free View",
	"Weapon Origin"
};

static const int VIEWORIGIN_WEAPON = 1;

wxBEGIN_EVENT_TABLE( CMainPanel, wxPanel )
	EVT_NOTEBOOK_PAGE_CHANGED( wxID_MAIN_PAGECHANGED, CMainPanel::PageChanged )
	EVT_RADIOBOX( wxID_MAIN_VIEWORIGINCHANGED, CMainPanel::ViewOriginChanged )
wxEND_EVENT_TABLE()

CMainPanel::CMainPanel( wxWindow* pParent, CHLMV* const pHLMV )
	: wxPanel( pParent )
	, m_pHLMV( pHLMV )
{
	wxASSERT( pHLMV != nullptr );

	m_p3DView = new C3DView( this, m_pHLMV, this );

	m_pControlPanel = new wxPanel( this );

	m_pMainControlBar = new wxPanel( m_pControlPanel );

	m_pViewOrigin = new wxRadioBox( m_pMainControlBar, wxID_MAIN_VIEWORIGINCHANGED, "View Origin", wxDefaultPosition, wxDefaultSize, 
									wxArrayString( ARRAYSIZE( VIEWORIGINS ), VIEWORIGINS ), 2, wxRA_SPECIFY_COLS );

	m_pDrawnPolys = new wxStaticText( m_pMainControlBar, wxID_ANY, "Drawn Polys: Undefined" );

	m_pFPS = new wxStaticText( m_pMainControlBar, wxID_ANY, "FPS: 0" );

	m_pControlPanels = new wxNotebook( m_pControlPanel, wxID_MAIN_PAGECHANGED );

	m_pModelDisplay = new CModelDisplayPanel( m_pControlPanels, m_pHLMV );

	m_pBodyParts = new CBodyPartsPanel( m_pControlPanels, m_pHLMV );

	m_pTextures = new CTexturesPanel( m_pControlPanels, m_pHLMV );

	m_pSequencesPanel = new CSequencesPanel( m_pControlPanels, m_pHLMV );

	m_pFullscreen = new CFullscreenPanel( m_pControlPanels, m_pHLMV );

	CBaseControlPanel* const panels[] = 
	{
		m_pModelDisplay,
		m_pBodyParts,
		m_pTextures,
		m_pSequencesPanel,
		m_pFullscreen
	};

	for( auto& pPanel : panels )
	{
		m_pControlPanels->AddPage( pPanel, pPanel->GetPanelName() );
	}

	m_pControlPanels->SetSelection( 0 );

	//Layout
	wxGridBagSizer* pBarSizer = new wxGridBagSizer( 5, 5 );

	pBarSizer->Add( m_pViewOrigin, wxGBPosition( 0, 0 ), wxGBSpan( 1, 2 ) );
	pBarSizer->Add( m_pDrawnPolys, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER );
	pBarSizer->Add( m_pFPS, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER );

	m_pMainControlBar->SetSizer( pBarSizer );

	wxBoxSizer* pPanelSizer = new wxBoxSizer( wxVERTICAL );

	pPanelSizer->Add( m_pMainControlBar, wxSizerFlags().Expand() );

	pPanelSizer->Add( m_pControlPanels, wxSizerFlags().Expand() );

	m_pControlPanel->SetSizer( pPanelSizer );

	wxBoxSizer* pSizer = new wxBoxSizer( wxVERTICAL );

	//3D view takes up 3/4th of the main area
	pSizer->Add( m_p3DView, wxSizerFlags().Expand().Proportion( 3 ) );
	pSizer->Add( m_pControlPanel, wxSizerFlags().Expand().Proportion( 1 ) );

	this->SetSizer( pSizer );

	InitializeUI();
}

CMainPanel::~CMainPanel()
{
}

void CMainPanel::RunFrame()
{
	++m_uiCurrentFPS;

	const long long iCurrentTick = GetCurrentTick();

	ForEachPanel( &CBaseControlPanel::ViewPreUpdate );

	m_p3DView->UpdateView();

	//Don't update if it's identical. Prevents flickering.
	if( m_uiOldDrawnPolys != m_pHLMV->GetState()->drawnPolys )
	{
		m_uiOldDrawnPolys = m_pHLMV->GetState()->drawnPolys;
		m_pDrawnPolys->SetLabelText( wxString::Format( "Drawn Polys: %u", m_pHLMV->GetState()->drawnPolys ) );
	}

	//Update FPS.
	if( iCurrentTick - m_iLastFPSUpdate >= 1000 )
	{
		m_iLastFPSUpdate = iCurrentTick;

		m_pFPS->SetLabelText( wxString::Format( "FPS: %u", m_uiCurrentFPS ) );

		m_uiCurrentFPS = 0;
	}

	ForEachPanel( &CBaseControlPanel::ViewUpdated );
}

void CMainPanel::Draw3D( const wxSize& size )
{
	const int iPage = m_pControlPanels->GetSelection();

	if( iPage != wxNOT_FOUND )
	{
		CBaseControlPanel* const pPage = static_cast<CBaseControlPanel*>( m_pControlPanels->GetPage( iPage ) );

		pPage->Draw3D( size );
	}
}

bool CMainPanel::LoadModel( const wxString& szFilename )
{
	m_p3DView->PrepareForLoad();

	m_pHLMV->GetState()->ResetModelData();

	m_pHLMV->GetState()->ClearStudioModel();

	auto szCFilename = szFilename.char_str( wxMBConvUTF8() );

	std::unique_ptr<StudioModel> pStudioModel = std::make_unique<StudioModel>();

	const StudioModel::LoadResult result = pStudioModel->Load( szFilename.c_str() );

	switch( result )
	{
	default:
	case StudioModel::LoadResult::FAILURE:
		{
			wxMessageBox( wxString::Format( "Error loading model \"%s\"\n", szCFilename.data() ), "Error" );
			return false;
		}

	case StudioModel::LoadResult::POSTLOADFAILURE:
		{
			wxMessageBox( wxString::Format( "Error post-loading model \"%s\"\n", szCFilename.data() ), "Error" );
			return false;
		}

	case StudioModel::LoadResult::VERSIONDIFFERS:
		{
			wxMessageBox( wxString::Format( "Error loading model \"%s\": version differs\n", szCFilename.data() ), "Error" );
			return false;
		}

	case StudioModel::LoadResult::SUCCESS: break;
	}

	m_pHLMV->GetState()->SetStudioModel( pStudioModel.release() );

	studiomodel::CStudioModel* pModel;

	const auto res = studiomodel::LoadStudioModel( szFilename.c_str(), true, pModel );

	switch( res )
	{
	default:
	case studiomodel::StudioModelLoadResult::FAILURE:
		{
			wxMessageBox( wxString::Format( "Error loading model \"%s\"\n", szCFilename.data() ), "Error" );
			return false;
		}

	case studiomodel::StudioModelLoadResult::POSTLOADFAILURE:
		{
			wxMessageBox( wxString::Format( "Error post-loading model \"%s\"\n", szCFilename.data() ), "Error" );
			return false;
		}

	case studiomodel::StudioModelLoadResult::VERSIONDIFFERS:
		{
			wxMessageBox( wxString::Format( "Error loading model \"%s\": version differs\n", szCFilename.data() ), "Error" );
			return false;
		}

	case studiomodel::StudioModelLoadResult::SUCCESS: break;
	}

	CStudioModelEntity* pEntity = static_cast<CStudioModelEntity*>( CBaseEntity::Create( "studiomodel", glm::vec3( 0, 50, 0 ), glm::vec3(), false ) );

	if( pEntity )
	{
		pEntity->SetModel( pModel );

		pEntity->Spawn();

		m_pHLMV->GetState()->SetModel( pEntity );
	}
	else
	{
		delete pModel;
	}

	InitializeUI();

	m_pHLMV->GetState()->CenterView();

	return true;
}

void CMainPanel::FreeModel()
{
	m_p3DView->PrepareForLoad();

	if( m_pHLMV->GetState()->GetStudioModel() )
	{
		m_pHLMV->GetState()->ClearStudioModel();
	}
}

void CMainPanel::InitializeUI()
{
	ForEachPanel( &CBaseControlPanel::InitializeUI );
}

void CMainPanel::PageChanged( wxBookCtrlEvent& event )
{
	const int iOldIndex = event.GetOldSelection();
	const int iNewIndex = event.GetSelection();

	if( iOldIndex != wxNOT_FOUND )
	{
		CBaseControlPanel* const pPage = static_cast<CBaseControlPanel*>( m_pControlPanels->GetPage( iOldIndex ) );

		pPage->PanelDeactivated();
	}

	if( iNewIndex != wxNOT_FOUND )
	{
		CBaseControlPanel* const pPage = static_cast<CBaseControlPanel*>( m_pControlPanels->GetPage( iNewIndex ) );

		pPage->PanelActivated();
	}
}

bool CMainPanel::LoadBackgroundTexture( const wxString& szFilename )
{
	return m_p3DView->LoadBackgroundTexture( szFilename );
}

void CMainPanel::UnloadBackgroundTexture()
{
	return m_p3DView->UnloadBackgroundTexture();
}

bool CMainPanel::LoadGroundTexture( const wxString& szFilename )
{
	return m_p3DView->LoadGroundTexture( szFilename );
}

void CMainPanel::UnloadGroundTexture()
{
	m_p3DView->UnloadGroundTexture();
}

void CMainPanel::SaveUVMap( const wxString& szFilename, const int iTexture )
{
	m_p3DView->SaveUVMap( szFilename, iTexture );
}

void CMainPanel::TakeScreenshot()
{
	m_p3DView->TakeScreenshot();
}

void CMainPanel::ViewOriginChanged( wxCommandEvent& event )
{
	m_pHLMV->GetState()->useWeaponOrigin = m_pViewOrigin->GetSelection() == VIEWORIGIN_WEAPON;
}
}