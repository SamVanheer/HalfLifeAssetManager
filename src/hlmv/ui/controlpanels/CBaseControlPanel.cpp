#include "CBaseControlPanel.h"

CBaseControlPanel::CBaseControlPanel( wxWindow* pParent, const wxString& szName, hlmv::CHLMV* const pHLMV )
	: wxPanel( pParent )
	, m_szName( szName )
	, m_pHLMV( pHLMV )
{
	wxASSERT( !szName.IsEmpty() );
	wxASSERT( pHLMV != nullptr );

	m_pName = new wxStaticText( this, wxID_ANY, szName );

	m_pBox = new wxStaticBox( this, wxID_ANY, "" );

	//Layout
	m_pBoxSizer = new wxStaticBoxSizer( m_pBox, wxVERTICAL );

	m_pBoxSizer->Add( m_pName );

	this->SetSizer( m_pBoxSizer );
}

CBaseControlPanel::~CBaseControlPanel()
{
}