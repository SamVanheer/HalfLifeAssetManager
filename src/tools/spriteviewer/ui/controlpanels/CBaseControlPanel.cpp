#include "CBaseControlPanel.h"

namespace sprview
{
CBaseControlPanel::CBaseControlPanel( wxWindow* pParent, const wxString& szName, CSpriteViewerApp* const pHLSV )
	: wxPanel( pParent )
	, m_szName( szName )
	, m_pHLSV( pHLSV )
{
	wxASSERT( !szName.IsEmpty() );
	wxASSERT( pHLSV != nullptr );

	//Layout
	m_pMainSizer = new wxBoxSizer( wxVERTICAL );

	this->SetSizer( m_pMainSizer );
}

CBaseControlPanel::~CBaseControlPanel()
{
}
}