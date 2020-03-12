#include <wx/sizer.h>

#include "CModelViewerApp.h"
#include "../CHLMVState.h"

#include "C3DView.h"

#include "CFullscreenWindow.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CFullscreenWindow, wxFrame )
	EVT_CHAR_HOOK( CFullscreenWindow::KeyDown )
wxEND_EVENT_TABLE()

CFullscreenWindow::CFullscreenWindow( CModelViewerApp* const pHLMV, C3DView* const view3D)
	: wxFrame( nullptr, wxID_ANY, HLMV_TITLE )
	, m_pHLMV( pHLMV )
{
	SetIcon( m_pHLMV->GetToolIcon() );

	m_p3DView = view3D;

	m_p3DView->Reparent(this);

	wxBoxSizer* pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( m_p3DView, wxSizerFlags().Expand().Proportion( 1 ) );

	this->SetSizer( pSizer );

	this->ShowFullScreen( true );
	this->SetFocus();
}

CFullscreenWindow::~CFullscreenWindow() = default;

void CFullscreenWindow::RunFrame()
{
	m_p3DView->UpdateView();
}

void CFullscreenWindow::KeyDown( wxKeyEvent& event )
{
	if( event.GetKeyCode() == WXK_ESCAPE )
	{
		Close( true );
	}
	else
	{
		event.Skip();
	}
}
}