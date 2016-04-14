#include <wx/gbsizer.h>

#include "CGeneralOptions.h"

namespace hlmv
{
CGeneralOptions::CGeneralOptions( wxWindow* pParent, CHLMVSettings* const pSettings )
	: wxPanel( pParent )
	, m_pSettings( pSettings )
{
	wxASSERT( pSettings );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	this->SetSizer( pSizer );
}

CGeneralOptions::~CGeneralOptions()
{
}

void CGeneralOptions::Save()
{
}
}