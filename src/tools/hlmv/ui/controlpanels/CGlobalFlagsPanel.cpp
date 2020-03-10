#include "../CModelViewerApp.h"

#include "shared/studiomodel/studio.h"

#include "wx/utility/wxUtil.h"

#include "CGlobalFlagsPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CGlobalFlagsPanel, CBaseControlPanel )
	EVT_CHECKBOX( wxID_GLOBALFLAGS_CHECKBOX, CGlobalFlagsPanel::CheckBoxChanged )
wxEND_EVENT_TABLE()

CGlobalFlagsPanel::CGlobalFlagsPanel( wxWindow* pParent, CModelViewerApp* const pHLMV )
	: CBaseControlPanel( pParent, "Global Flags", pHLMV )
{
	//Helps catch errors if we miss one.
	memset( m_pCheckBoxes, 0, sizeof( m_pCheckBoxes ) );

	wxWindow* const pElemParent = GetElementParent();

	m_pCheckBoxes[ CheckBox::ROCKET ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Rocket Trail" );
	m_pCheckBoxes[ CheckBox::GRENADE ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Grenade Smoke" );
	m_pCheckBoxes[ CheckBox::GIB ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Gib Blood" );
	m_pCheckBoxes[ CheckBox::ROTATE ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Modle Rotate" );
	m_pCheckBoxes[ CheckBox::TRACER ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Green Trail" );

	m_pCheckBoxes[ CheckBox::ZOMGIB ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Zombie Blood" );
	m_pCheckBoxes[ CheckBox::TRACER2 ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Orange Trail" );
	m_pCheckBoxes[ CheckBox::TRACER3 ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Purple Trail" );
	m_pCheckBoxes[ CheckBox::NOSHADELIGHT ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "No Shade Light" );
	m_pCheckBoxes[ CheckBox::HITBOXCOLLISIONS ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Hitbox Collision" );
	m_pCheckBoxes[ CheckBox::FORCESKYLIGHT ] = new wxCheckBox( pElemParent, wxID_GLOBALFLAGS_CHECKBOX, "Force Skylight" );

	for( size_t uiIndex = CheckBox::FIRST; uiIndex < CheckBox::COUNT; ++uiIndex )
	{
		wxASSERT( m_pCheckBoxes[ uiIndex ] );

		m_pCheckBoxes[ uiIndex ]->SetClientData( reinterpret_cast<void*>( uiIndex ) );
	}

	//Layout
	auto pSizer = new wxBoxSizer( wxHORIZONTAL );

	pSizer->Add( new wxStaticText( pElemParent, wxID_ANY, "Global model flags:" ) );

	auto pCheckBoxSizer = wx::CreateCheckBoxSizer( m_pCheckBoxes, ARRAYSIZE( m_pCheckBoxes ), NUM_CHECKBOX_COLS, wxEXPAND );

	pSizer->Add( pCheckBoxSizer, wxSizerFlags().Expand().Border() );

	GetMainSizer()->Add( pSizer );
}

CGlobalFlagsPanel::~CGlobalFlagsPanel()
{
}

void CGlobalFlagsPanel::InitializeUI()
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	bool bInitialized = false;

	if( pEntity )
	{
		auto pModel = pEntity->GetModel();

		if( pModel )
		{
			const studiohdr_t* const pHdr = pModel->GetStudioHeader();

			if( pHdr )
			{
				for( size_t uiIndex = CheckBox::FIRST; uiIndex < CheckBox::COUNT; ++uiIndex )
				{
					m_pCheckBoxes[ uiIndex ]->SetValue( ( pHdr->flags & static_cast<int>( 1 << uiIndex ) ) != 0 );
				}

				bInitialized = true;
			}
		}
	}

	if( !bInitialized )
	{
		for( size_t uiIndex = CheckBox::FIRST; uiIndex < CheckBox::COUNT; ++uiIndex )
		{
			m_pCheckBoxes[ uiIndex ]->SetValue( false );
		}
	}
}

void CGlobalFlagsPanel::SetCheckBox( const CheckBox::Type checkBox, const bool bValue )
{
	InternalSetCheckBox( checkBox, bValue, false );
}

void CGlobalFlagsPanel::InternalSetCheckBox( const CheckBox::Type checkBox, const bool bValue, const bool bCameFromChangeEvent )
{
	if( checkBox < CheckBox::FIRST || checkBox > CheckBox::LAST )
		return;

	wxCheckBox* pCheckBox = m_pCheckBoxes[ checkBox ];

	//Don't do anything if it's identical. Helps prevent unnecessary calls.
	if( !bCameFromChangeEvent && pCheckBox->GetValue() == bValue )
		return;

	pCheckBox->SetValue( bValue );

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	auto pModel = pEntity->GetModel();

	if( !pModel )
		return;

	studiohdr_t* const pHdr = pModel->GetStudioHeader();

	if( !pHdr )
		return;

	if( bValue )
		pHdr->flags |= static_cast<int>( 1 << checkBox );
	else
		pHdr->flags &= ~static_cast<int>( 1 << checkBox );

	m_pHLMV->GetState()->modelChanged = true;
}

void CGlobalFlagsPanel::CheckBoxChanged( wxCommandEvent& event )
{
	wxCheckBox* const pCheckBox = static_cast<wxCheckBox*>( event.GetEventObject() );

	const CheckBox::Type checkbox = static_cast<CheckBox::Type>( reinterpret_cast<int>( pCheckBox->GetClientData() ) );

	if( checkbox < CheckBox::FIRST || checkbox > CheckBox::LAST )
		return;

	InternalSetCheckBox( checkbox, pCheckBox->GetValue(), true );
}
}