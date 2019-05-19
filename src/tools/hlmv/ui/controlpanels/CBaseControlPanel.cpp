#include "CBaseControlPanel.h"

namespace hlmv
{
CBaseControlPanel::CBaseControlPanel( wxWindow* pParent, const wxString& szName, CModelViewerApp* const pHLMV )
	: wxScrolledWindow( pParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL )
	, m_szName( szName )
	, m_pHLMV( pHLMV )
{
	wxASSERT( !szName.IsEmpty() );
	wxASSERT( pHLMV != nullptr );

	//Layout
	m_pMainSizer = new wxBoxSizer( wxVERTICAL );

	this->SetSizer( m_pMainSizer );

	SetScrollRate( 5, 5 );
}

CBaseControlPanel::~CBaseControlPanel()
{
}
}