#include <vector>

#include <glm/vec3.hpp>

#include "shared/Const.h"

#include "engine/shared/studiomodel/studio.h"
#include "engine/shared/studiomodel/CStudioModel.h"

#include "CStudioTypesCheatSheet.h"

namespace ui
{
CStudioTypesCheatSheet::CStudioTypesCheatSheet( wxWindow* pParent )
	: wxDialog( pParent, wxID_ANY, "Types Cheat Sheet" )
{
	std::vector<wxWindow*> texts;

	for( int iType = STUDIO_CONTROL_FIRST; iType <= STUDIO_CONTROL_LAST; iType <<= 1 )
	{
		texts.push_back( new wxStaticText( this, wxID_ANY, wxString::Format( "%s: %s", studiomdl::ControlToString( iType ), studiomdl::ControlToStringDescription( iType ) ) ) );
	}

	auto pOk = this->CreateSeparatedButtonSizer( wxOK );

	//Layout
	auto pSizer = new wxBoxSizer( wxVERTICAL );

	for( auto pText : texts )
		pSizer->Add( pText, wxSizerFlags().Expand().DoubleBorder() );

	pSizer->Add( pOk, wxSizerFlags().Expand() );

	this->SetSizer( pSizer );

	this->Fit();

	this->CenterOnScreen();
}
}