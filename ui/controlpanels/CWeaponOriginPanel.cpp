#include <cmath>

#include <wx/gbsizer.h>

#include "model/options/COptions.h"

#include "CWeaponOriginPanel.h"

wxBEGIN_EVENT_TABLE( CWeaponOriginPanel, CBaseSequencesPanel )
	EVT_BUTTON( wxID_WPN_TESTORIGINS, CWeaponOriginPanel::TestOrigins )
wxEND_EVENT_TABLE()

CWeaponOriginPanel::CWeaponOriginPanel( wxWindow* pParent )
	: CBaseSequencesPanel( pParent, "Weapon Origin" )
{
	wxWindow* const pElemParent = GetBox();

	//TODO: use float only controls here
	m_pXOrigin = new wxTextCtrl( pElemParent, wxID_ANY, "Undefined" );
	m_pYOrigin = new wxTextCtrl( pElemParent, wxID_ANY, "Undefined" );
	m_pZOrigin = new wxTextCtrl( pElemParent, wxID_ANY, "Undefined" );

	wxStaticText* pXOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin X" );
	wxStaticText* pYOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin Y" );
	wxStaticText* pZOrigin = new wxStaticText( pElemParent, wxID_ANY, "Origin Z" );

	m_pTestOrigins = new wxButton( pElemParent, wxID_WPN_TESTORIGINS, "Test Origins" );

	m_pShowCrosshair = new wxCheckBox( pElemParent, wxID_ANY, "Show Crosshair" );
	m_pShowGuidelines = new wxCheckBox( pElemParent, wxID_ANY, "Show Guidelines" );

	wxStaticText* pOriginInfo = new wxStaticText( pElemParent, wxID_ANY, "Changes to the origin must be\nmade by altering the $origin line\nin your model's QC file." );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	CreateUI( pSizer );

	pSizer->Add( m_pXOrigin, wxGBPosition( 0, BASESEQUENCES_FIRST_FREE_COL ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pYOrigin, wxGBPosition( 1, BASESEQUENCES_FIRST_FREE_COL ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pZOrigin, wxGBPosition( 2, BASESEQUENCES_FIRST_FREE_COL ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( pXOrigin, wxGBPosition( 0, BASESEQUENCES_FIRST_FREE_COL + 1 ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( pYOrigin, wxGBPosition( 1, BASESEQUENCES_FIRST_FREE_COL + 1 ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( pZOrigin, wxGBPosition( 2, BASESEQUENCES_FIRST_FREE_COL + 1 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pTestOrigins, wxGBPosition( 3, BASESEQUENCES_FIRST_FREE_COL ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pShowCrosshair, wxGBPosition( 0, BASESEQUENCES_FIRST_FREE_COL + 2 ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pShowGuidelines, wxGBPosition( 1, BASESEQUENCES_FIRST_FREE_COL + 2 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( pOriginInfo, wxGBPosition( 2, BASESEQUENCES_FIRST_FREE_COL + 2 ), wxGBSpan( 2, 1 ), wxEXPAND );

	GetBoxSizer()->Add( pSizer );
}

CWeaponOriginPanel::~CWeaponOriginPanel()
{
}

void CWeaponOriginPanel::Draw3D( const wxSize& size )
{
	const int x = size.GetX() / 2;
	const int y = size.GetY() / 2;

	if( m_pShowCrosshair->GetValue() )
	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		glOrtho( 0.0f, ( float ) size.GetX(), ( float ) size.GetY(), 0.0f, 1.0f, -1.0f );

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();

		glDisable( GL_CULL_FACE );

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		glDisable( GL_TEXTURE_2D );

		glColor4f( 1.0, 0.0, 0.0, 1.0 );

		glPointSize( CROSSHAIR_LINE_WIDTH );
		glLineWidth( CROSSHAIR_LINE_WIDTH );

		glBegin( GL_POINTS );

		glVertex2f( x - CROSSHAIR_LINE_WIDTH / 2, y + 1 );

		glEnd();

		glBegin( GL_LINES );

		glVertex2f( x - CROSSHAIR_LINE_START, y );
		glVertex2f( x - CROSSHAIR_LINE_END, y );

		glVertex2f( x + CROSSHAIR_LINE_START, y );
		glVertex2f( x + CROSSHAIR_LINE_END, y );

		glVertex2f( x, y - CROSSHAIR_LINE_START );
		glVertex2f( x, y - CROSSHAIR_LINE_END );

		glVertex2f( x, y + CROSSHAIR_LINE_START );
		glVertex2f( x, y + CROSSHAIR_LINE_END );

		glEnd();

		glPointSize( 1 );
		glLineWidth( 1 );

		glPopMatrix();
	}

	if( m_pShowGuidelines->GetValue() )
	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		glOrtho( 0.0f, ( float ) size.GetX(), ( float ) size.GetY(), 0.0f, 1.0f, -1.0f );

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();

		glDisable( GL_CULL_FACE );

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		glDisable( GL_TEXTURE_2D );

		glColor4f( 1.0, 0.0, 0.0, 1.0 );

		glPointSize( GUIDELINES_LINE_WIDTH );
		glLineWidth( GUIDELINES_LINE_WIDTH );

		glBegin( GL_POINTS );

		for( int yPos = size.GetY() - GUIDELINES_LINE_LENGTH; yPos >= y + CROSSHAIR_LINE_END; yPos -= GUIDELINES_OFFSET )
		{
			glVertex2f( x - GUIDELINES_LINE_WIDTH, yPos );
		}

		glEnd();

		glBegin( GL_LINES );

		for( int yPos = size.GetY() - GUIDELINES_LINE_LENGTH - GUIDELINES_POINT_LINE_OFFSET - GUIDELINES_LINE_WIDTH; 
			 yPos >= y + CROSSHAIR_LINE_END + GUIDELINES_LINE_LENGTH; 
			 yPos -= GUIDELINES_OFFSET )
		{
			glVertex2f( x, yPos );
			glVertex2f( x, yPos - GUIDELINES_LINE_LENGTH );
		}

		glEnd();

		const float flWidth = size.GetY() * ( 16 / 9.0 );

		glLineWidth( GUIDELINES_EDGE_WIDTH );

		glBegin( GL_LINES );

		glVertex2f( ( size.GetX() / 2 ) - ( flWidth / 2 ), 0 );
		glVertex2f( ( size.GetX() / 2 ) - ( flWidth / 2 ), size.GetY() );

		glVertex2f( ( size.GetX() / 2 ) + ( flWidth / 2 ), 0 );
		glVertex2f( ( size.GetX() / 2 ) + ( flWidth / 2 ), size.GetY() );

		glEnd();

		glPointSize( 1 );
		glLineWidth( 1 );

		glPopMatrix();
	}
}

void CWeaponOriginPanel::ModelChanged( const StudioModel& model )
{
	CBaseSequencesPanel::ModelChanged( model );

	//TODO: fill info
	m_pXOrigin->SetLabelText( wxString::Format( "%.6f", Options.weaponOrigin[ 0 ] ) );
	m_pYOrigin->SetLabelText( wxString::Format( "%.6f", Options.weaponOrigin[ 1 ] ) );
	m_pZOrigin->SetLabelText( wxString::Format( "%.6f", Options.weaponOrigin[ 2 ] ) );
}

void CWeaponOriginPanel::PanelActivated()
{
	Options.useWeaponOrigin = true;
}

void CWeaponOriginPanel::PanelDeactivated()
{
	Options.useWeaponOrigin = false;
}

static void GetDoubleFromTextCtrl( wxTextCtrl* const pCtrl, vec_t& flInOutValue )
{
	double flValue = 0;

	if( pCtrl->GetValue().ToDouble( &flValue ) )
		flInOutValue = flValue;
	else
	{
		//Invalid content, reset.
		pCtrl->ChangeValue( wxString::Format( "%.6f", flInOutValue ) );
	}
}

void CWeaponOriginPanel::TestOrigins( wxCommandEvent& event )
{
	double flValue = 0;

	GetDoubleFromTextCtrl( m_pXOrigin, Options.weaponOrigin[ 0 ] );
	GetDoubleFromTextCtrl( m_pYOrigin, Options.weaponOrigin[ 1 ] );
	GetDoubleFromTextCtrl( m_pZOrigin, Options.weaponOrigin[ 2 ] );
}