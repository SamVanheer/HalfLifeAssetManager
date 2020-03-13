#include <memory>

#include <wx/gbsizer.h>
#include <wx/collpane.h>

#include "C3DView.h"

#include "controlpanels/CModelDisplayPanel.h"
#include "controlpanels/CModelDataPanel.h"
#include "controlpanels/CBodyPartsPanel.h"
#include "controlpanels/CBonesPanel.h"
#include "controlpanels/CAttachmentsPanel.h"
#include "controlpanels/CHitboxesPanel.h"
#include "controlpanels/CTexturesPanel.h"
#include "controlpanels/CSequencesPanel.h"
#include "controlpanels/CGlobalFlagsPanel.h"

#include "shared/studiomodel/CStudioModel.h"
#include "shared/renderer/studiomodel/IStudioModelRenderer.h"
#include "game/entity/CStudioModelEntity.h"
#include "game/entity/CBaseEntityList.h"

#include "CFullscreenWindow.h"
#include "CModelViewerApp.h"
#include "../CHLMVState.h"

#include "CMainPanel.h"

//TODO: remove
extern studiomdl::IStudioModelRenderer* g_pStudioMdlRenderer;

namespace hlmv
{
static const wxString VIEWORIGINS[] = 
{
	"Free View",
	"Weapon Origin"
};

static const int VIEWORIGIN_WEAPON = 1;

const glm::vec3 CMainPanel::DEFAULT_LIGHT_VECTOR{ 0, 0, -1 };

wxBEGIN_EVENT_TABLE( CMainPanel, wxPanel )
	EVT_NOTEBOOK_PAGE_CHANGED( wxID_MAIN_PAGECHANGED, CMainPanel::PageChanged )
	EVT_RADIOBOX( wxID_MAIN_VIEWORIGINCHANGED, CMainPanel::ViewOriginChanged )
	EVT_BUTTON( wxID_MAIN_RESETLIGHTVECTOR, CMainPanel::ResetLightVector )
wxEND_EVENT_TABLE()

CMainPanel::CMainPanel( wxWindow* pParent, CModelViewerApp* const pHLMV )
	: wxPanel( pParent )
	, m_pHLMV( pHLMV )
{
	wxASSERT( pHLMV != nullptr );

	m_pControlPanel = new wxPanel( this );

	m_pMainControlBar = new wxPanel( m_pControlPanel );

	m_pViewOrigin = new wxRadioBox( m_pMainControlBar, wxID_MAIN_VIEWORIGINCHANGED, "View Origin", wxDefaultPosition, wxDefaultSize, 
									wxArrayString( ARRAYSIZE( VIEWORIGINS ), VIEWORIGINS ), 2, wxRA_SPECIFY_COLS );

	m_pGoFullscreen = new wxButton(m_pMainControlBar, wxID_ANY, "Fullscreen");
	m_pGoFullscreen->Bind(wxEVT_BUTTON, &CMainPanel::OnGoFullscreen, this);

	m_pDrawnPolys = new wxStaticText( m_pMainControlBar, wxID_ANY, "Drawn Polys: Undefined" );

	m_pFPS = new wxStaticText( m_pMainControlBar, wxID_ANY, "FPS: 0" );

	m_pLightVector = new wxStaticText( m_pMainControlBar, wxID_ANY, "Light Vector: 0 0 0" );

	m_pResetLightVector = new wxButton( m_pMainControlBar, wxID_MAIN_RESETLIGHTVECTOR, "Reset Light Vector" );

	m_pControlPanels = new wxNotebook( m_pControlPanel, wxID_MAIN_PAGECHANGED );

	m_p3DView = new C3DView(this, m_pHLMV, this);

	m_pModelDisplay = new CModelDisplayPanel( m_pControlPanels, m_pHLMV );

	m_pModelData = new CModelDataPanel(m_pControlPanels, m_pHLMV);

	m_pGlobalFlags = new CGlobalFlagsPanel( m_pControlPanels, m_pHLMV );

	m_pBodyParts = new CBodyPartsPanel( m_pControlPanels, m_pHLMV );

	m_pBones = new CBonesPanel( m_pControlPanels, m_pHLMV );

	m_pAttachments = new CAttachmentsPanel( m_pControlPanels, m_pHLMV );

	m_pHitboxes = new CHitboxesPanel(m_pControlPanels, m_pHLMV);

	m_pTextures = new CTexturesPanel( m_pControlPanels, m_pHLMV );

	m_pSequencesPanel = new CSequencesPanel( m_pControlPanels, m_pHLMV );

	CBaseControlPanel* const panels[] = 
	{
		m_pModelDisplay,
		m_pModelData,
		m_pGlobalFlags,
		m_pBodyParts,
		m_pBones,
		m_pAttachments,
		m_pHitboxes,
		m_pTextures,
		m_pSequencesPanel
	};

	for( auto& pPanel : panels )
	{
		m_pControlPanels->AddPage( pPanel, pPanel->GetPanelName() );
	}

	m_pControlPanels->SetSelection( 0 );

	//Layout
	wxGridBagSizer* pBarSizer = new wxGridBagSizer(1, 1);

	int iRow = 0;

	pBarSizer->Add( m_pViewOrigin, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 2 ) );
	pBarSizer->Add( m_pDrawnPolys, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL );
	pBarSizer->Add( m_pFPS, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL );
	pBarSizer->Add( m_pLightVector, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL | wxEXPAND );
	pBarSizer->Add( m_pResetLightVector, wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ), wxEXPAND);
	pBarSizer->Add(m_pGoFullscreen, wxGBPosition(iRow++, 1), wxGBSpan(1, 1), wxEXPAND);

	m_pMainControlBar->SetSizer( pBarSizer );

	wxBoxSizer* pPanelSizer = new wxBoxSizer( wxHORIZONTAL );

	//Add a 10px border on the left so the text isn't pasted against the window border.
	pPanelSizer->AddSpacer( 10 );

	pPanelSizer->Add( m_pMainControlBar, wxSizerFlags().Expand() );

	//Padding.
	pPanelSizer->AddSpacer( 10 );

	pPanelSizer->Add( m_pControlPanels, wxSizerFlags().Expand().Proportion( 1 ) );

	m_pControlPanel->SetSizer( pPanelSizer );

	wxBoxSizer* pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( m_p3DView, wxSizerFlags().Expand().Proportion( 1 ) );
	pSizer->Add( m_pControlPanel, wxSizerFlags().Expand().Proportion( 0 ) );

	this->SetSizer( pSizer );

	//Force the control panel to be a little bigger than it gets laid out to be
	//so it doesn't create vertical scroll bars
	auto controlPanelSize = m_pControlPanel->GetMinSize();
	controlPanelSize.SetHeight(140);
	m_pControlPanel->SetMinClientSize(controlPanelSize);

	InitializeUI();

	g_pStudioMdlRenderer->SetRendererListener( this );
}

CMainPanel::~CMainPanel()
{
	g_pStudioMdlRenderer->SetRendererListener( nullptr );
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

	const glm::vec3 vecLight = g_pStudioMdlRenderer->GetLightVector();

	if( m_vecPrevLightVec != vecLight )
	{
		m_vecPrevLightVec = vecLight;

		m_pLightVector->SetLabelText( wxString::Format( "Light Vector: %.2f %.2f %.2f", m_vecPrevLightVec[ 0 ], m_vecPrevLightVec[ 1 ], m_vecPrevLightVec[ 2 ] ) );
	}

	ForEachPanel( &CBaseControlPanel::ViewUpdated );
}

bool CMainPanel::LoadModel( const wxString& szFilename )
{
	m_p3DView->PrepareForLoad();

	m_pHLMV->GetState()->ResetModelData();

	m_pHLMV->GetState()->ClearEntity();

	auto szCFilename = szFilename.char_str( wxMBConvUTF8() );

	try
	{
		auto pModel = studiomdl::LoadStudioModel(szFilename.c_str());
	
		CHLMVStudioModelEntity* pEntity = static_cast<CHLMVStudioModelEntity*>( CBaseEntity::Create( "studiomodel", glm::vec3(), glm::vec3(), false ) );

		if( pEntity )
		{
			pEntity->m_pState = m_pHLMV->GetState();

			pEntity->SetModel( pModel.release() );

			pEntity->Spawn();

			m_pHLMV->GetState()->SetEntity( pEntity );
		}

		InitializeUI();

		m_pHLMV->GetState()->CenterView();

		return true;
	}
	catch (const studiomdl::StudioModelException& e)
	{
		wxMessageBox(wxString::Format("Error loading model \"%s\":\n%s", szCFilename.data(), e.what()), "Error");
	}

	return false;
}

void CMainPanel::FreeModel()
{
	m_p3DView->PrepareForLoad();

	m_pHLMV->GetState()->ClearEntity();
}

void CMainPanel::InitializeUI()
{
	ForEachPanel( &CBaseControlPanel::InitializeUI );

	g_pStudioMdlRenderer->SetLightVector( DEFAULT_LIGHT_VECTOR );
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

void CMainPanel::OnPostDraw( studiomdl::IStudioModelRenderer& renderer, const studiomdl::CModelRenderInfo& info )
{
	auto pPage = static_cast<CBaseControlPanel*>( m_pControlPanels->GetCurrentPage() );

	if( pPage )
		pPage->OnPostDraw( renderer, info );
}

void CMainPanel::ViewOriginChanged( wxCommandEvent& event )
{
	m_pHLMV->GetState()->SetUseWeaponOrigin( m_pViewOrigin->GetSelection() == VIEWORIGIN_WEAPON );
}

void CMainPanel::ResetLightVector( wxCommandEvent& event )
{
	g_pStudioMdlRenderer->SetLightVector( DEFAULT_LIGHT_VECTOR );
}

void CMainPanel::OnGoFullscreen(wxCommandEvent& event)
{
	if (m_pHLMV->GetFullscreenWindow())
	{
		wxMessageBox("A fullscreen window is already open!");
		return;
	}

	m_p3DView->GetContainingSizer()->Detach(m_p3DView);

	auto window = new CFullscreenWindow(m_pHLMV, m_p3DView);

	window->Bind(wxEVT_CLOSE_WINDOW, &CMainPanel::OnCloseFullscreenWindow, this);

	m_pHLMV->SetFullscreenWindow(window);
}

void CMainPanel::OnCloseFullscreenWindow(wxCloseEvent& event)
{
	event.Skip();

	m_pHLMV->SetFullscreenWindow(nullptr);

	//Reparent the 3D view to this panel
	m_p3DView->Reparent(this);

	m_p3DView->GetContainingSizer()->Detach(m_p3DView);

	GetSizer()->Insert(0, m_p3DView, wxSizerFlags(1).Expand());

	Layout();
}
}