#include <wx/sizer.h>

#include "hlmv/ui/CModelViewerApp.h"

#include "hlmv/CHLMVSettings.h"

#include "C3DView.h"

#include "CFullscreenWindow.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CFullscreenWindow, wxFrame )
	EVT_KEY_DOWN( CFullscreenWindow::KeyDown )
wxEND_EVENT_TABLE()

CFullscreenWindow::CFullscreenWindow( hlmv::CHLMVState* const pSettings )
	: wxFrame( nullptr, wxID_ANY, HLMV_TITLE )
	, m_pSettings( pSettings )
{
	wxGetApp().SetFullscreenWindow( this );

	m_p3DView = new C3DView( this, m_pSettings );

	wxBoxSizer* pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( m_p3DView, wxSizerFlags().Expand().Proportion( 1 ) );

	this->SetSizer( pSizer );

	this->ShowFullScreen( true );
	this->SetFocus();

	this->Show( true );
}

CFullscreenWindow::~CFullscreenWindow()
{
	wxGetApp().SetFullscreenWindow( nullptr );
}

void CFullscreenWindow::OnTimer( CTimer& timer )
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
		event.Skip();
}
}