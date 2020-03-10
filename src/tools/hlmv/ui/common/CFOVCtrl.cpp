#include <cfloat>

#include <wx/spinctrl.h>
#include <wx/gbsizer.h>

#include "CFOVCtrl.h"

namespace ui
{
const wxString FOVCtrlNameStr = "CFOVCtrl";

wxDEFINE_EVENT( wxEVT_FOV_CHANGED, wxCommandEvent );

enum FOVCtrlId
{
	FOV_VALUE = wxID_HIGHEST + 1,
	FOV_SET_BUTTON,
	FOV_SET_DEFAULT,
};

wxBEGIN_EVENT_TABLE( CFOVCtrl, wxPanel )
	EVT_SPINCTRLDOUBLE( FOV_VALUE, CFOVCtrl::OnFOVChanged )
	EVT_BUTTON( FOV_SET_BUTTON, CFOVCtrl::OnSetFOV )
	EVT_BUTTON( FOV_SET_DEFAULT, CFOVCtrl::OnSetDefault )
wxEND_EVENT_TABLE()

CFOVCtrl::CFOVCtrl( wxWindow *parent,
			 wxWindowID winid,
			 const float flDefault,
			 const wxString& szLabelText,
			 const wxPoint& pos,
			 const wxSize& size,
			 long style,
			 const wxString& name )
	: wxPanel( parent, winid, pos, size, style, name )
{
	m_pText = new wxStaticText( this, wxID_ANY, szLabelText );
	m_pValue = new wxSpinCtrlDouble( this, FOV_VALUE, wxEmptyString, wxDefaultPosition, wxSize( 75, wxDefaultSize.GetHeight() ) );
	m_pSet = new wxButton( this, FOV_SET_BUTTON, "Set FOV", wxDefaultPosition, wxSize( 75, wxDefaultSize.GetHeight() ) );
	m_pDefault = new wxButton( this, FOV_SET_DEFAULT, "Default", wxDefaultPosition, wxSize( 75, wxDefaultSize.GetHeight() ) );

	//Because why not, maximum DBL value.
	m_pValue->SetRange( 0, DBL_MAX );
	m_pValue->SetDigits( 1 );

	//Layout
	auto pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( m_pText, wxGBPosition( 0, 0 ), wxGBSpan( 1, 3 ), wxALIGN_CENTER_VERTICAL );

	pSizer->Add( m_pValue, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL );
	pSizer->Add( m_pSet, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL );
	pSizer->Add( m_pDefault, wxGBPosition( 1, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL );

	this->SetSizer( pSizer );

	SetDefault( flDefault );

	//Avoid generating an event here.
	ChangeToDefault();
}

CFOVCtrl::~CFOVCtrl()
{
}


void CFOVCtrl::SetDefault( const float flDefault )
{
	m_flDefault = flDefault;
}

void CFOVCtrl::ChangeToDefault()
{
	ChangeValue( m_flDefault );
}

void CFOVCtrl::ResetToDefault()
{
	SetValue( m_flDefault );
}

float CFOVCtrl::GetValue() const
{
	return m_pValue->GetValue();
}

void CFOVCtrl::ChangeValue( const float flValue )
{
	m_pValue->SetValue( flValue );
}

void CFOVCtrl::SetValue( const float flValue )
{
	ChangeValue( flValue );

	ValueChanged();
}

void CFOVCtrl::ValueChanged()
{
	wxCommandEvent event( wxEVT_FOV_CHANGED, GetId() );
	event.SetEventObject( this );

	ProcessWindowEvent( event );
}

void CFOVCtrl::OnFOVChanged( wxSpinDoubleEvent& event )
{
	ValueChanged();
}

void CFOVCtrl::OnSetFOV( wxCommandEvent& event )
{
	ValueChanged();

	//Don't propagate this up.
	event.StopPropagation();
}

void CFOVCtrl::OnSetDefault( wxCommandEvent& event )
{
	ResetToDefault();

	//Don't propagate this up.
	event.StopPropagation();
}
}