#include "CBaseControlPanel.h"

namespace hlmv
{
CBaseControlPanel::CBaseControlPanel( wxWindow* pParent, const wxString& szName, CModelViewerApp* const pHLMV )
	: wxPanel( pParent )
	, m_szName( szName )
	, m_pHLMV( pHLMV )
{
	wxASSERT( !szName.IsEmpty() );
	wxASSERT( pHLMV != nullptr );

	//Layout
	m_pMainSizer = new wxBoxSizer( wxVERTICAL );

	this->SetSizer( m_pMainSizer );
}

CBaseControlPanel::~CBaseControlPanel()
{
}
}