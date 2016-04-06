#include <wx/notebook.h>

#include "C3DView.h"

#include "controlpanels/CModelDisplayPanel.h"
#include "controlpanels/CBodyPartsPanel.h"
#include "controlpanels/CTexturesPanel.h"
#include "controlpanels/CSequencesPanel.h"
#include "controlpanels/CWeaponOriginPanel.h"
#include "controlpanels/CFullscreenPanel.h"

#include "model/studiomodel/StudioModel.h"

#include "hlmv/CHLMVOptions.h"

#include "CMainPanel.h"

wxBEGIN_EVENT_TABLE( CMainPanel, wxPanel )
	EVT_NOTEBOOK_PAGE_CHANGED( wxID_MAIN_PAGECHANGED, CMainPanel::PageChanged )
wxEND_EVENT_TABLE()

CMainPanel::CMainPanel( wxWindow* pParent )
	: wxPanel( pParent )
{
	m_pTimer = new CTimer( this );

	//60 FPS
	m_pTimer->Start( ( 1 / 60.0 ) * 1000 );

	m_p3DView = new C3DView( this, this );

	m_pControlPanels = new wxNotebook( this, wxID_MAIN_PAGECHANGED );

	m_pModelDisplay = new CModelDisplayPanel( m_pControlPanels );

	m_pBodyParts = new CBodyPartsPanel( m_pControlPanels );

	m_pTextures = new CTexturesPanel( m_pControlPanels );

	m_pSequencesPanel = new CSequencesPanel( m_pControlPanels );

	m_pWeaponOriginPanel = new CWeaponOriginPanel( m_pControlPanels );

	m_pFullscreen = new CFullscreenPanel( m_pControlPanels );

	CBaseControlPanel* const panels[] = 
	{
		m_pModelDisplay,
		m_pBodyParts,
		m_pTextures,
		m_pSequencesPanel,
		m_pWeaponOriginPanel,
		m_pFullscreen
	};

	for( auto& pPanel : panels )
	{
		m_pControlPanels->AddPage( pPanel, pPanel->GetPanelName() );
	}

	m_pControlPanels->SetSelection( 0 );

	//Layout
	wxBoxSizer* pSizer = new wxBoxSizer( wxVERTICAL );

	//3D view takes up 3/4th of the main area
	pSizer->Add( m_p3DView, wxSizerFlags().Expand().Proportion( 3 ) );
	pSizer->Add( m_pControlPanels, wxSizerFlags().Expand().Proportion( 1 ) );

	this->SetSizer( pSizer );
}

CMainPanel::~CMainPanel()
{
	delete m_pTimer;
}

void CMainPanel::OnTimer( CTimer& timer )
{
	ForEachPanel( &CBaseControlPanel::ViewPreUpdate );

	m_p3DView->UpdateView();

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

	g_studioModel.FreeModel();

	Options.ResetModelData();

	auto szCFilename = szFilename.char_str( wxMBConvUTF8() );

	if( !g_studioModel.LoadModel( szCFilename ) )
	{
		g_studioModel.FreeModel();

		wxMessageBox( wxString::Format( "Error loading model \"%s\"\n", szCFilename.data() ), "Error" );
		return false;
	}

	if( !g_studioModel.PostLoadModel( szCFilename ) )
	{
		g_studioModel.FreeModel();

		wxMessageBox( wxString::Format( "Error post-loading model \"%s\"\n", szCFilename.data() ), "Error" );
		return false;
	}

	ModelChanged( g_studioModel );

	Options.CenterView( g_studioModel );

	return true;
}

void CMainPanel::ModelChanged( const StudioModel& model )
{
	ForEachPanel( &CBaseControlPanel::ModelChanged, model );
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

void CMainPanel::LoadBackgroundTexture( const wxString& szFilename )
{
	m_p3DView->LoadBackgroundTexture( szFilename );
}

void CMainPanel::LoadGroundTexture( const wxString& szFilename )
{
	m_p3DView->LoadGroundTexture( szFilename );
}

void CMainPanel::UnloadGroundTexture()
{
	m_p3DView->UnloadGroundTexture();
}

void CMainPanel::SaveUVMap( const wxString& szFilename, const int iTexture )
{
	m_p3DView->SaveUVMap( szFilename, iTexture );
}