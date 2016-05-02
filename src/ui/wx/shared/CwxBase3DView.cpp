#include "utility/mathlib.h"

#include "CwxBase3DView.h"

namespace ui
{
wxBEGIN_EVENT_TABLE( CwxBase3DView, CwxBaseGLCanvas )
	EVT_MOUSE_EVENTS( CwxBase3DView::MouseEvents )
wxEND_EVENT_TABLE()

CwxBase3DView::CwxBase3DView( wxWindow* pParent, I3DViewListener* pDrawListener, wxWindowID id,
			   const wxPoint& pos, const wxSize& size,
			   long style,
			   const wxString& name,
			   const wxPalette& palette )
	: CwxBaseGLCanvas( pParent, id, pos, size, style, name, palette )
	, m_pDrawListener( pDrawListener )
{
}

CwxBase3DView::~CwxBase3DView()
{
}

void CwxBase3DView::SetCamera( graphics::CCamera* pCamera )
{
	m_pCamera = pCamera;
	//Also set the old camera so no leftover data gets used.

	if( pCamera )
		m_OldCamera = *pCamera;
}

void CwxBase3DView::DrawScene()
{
	OnPreDraw();

	OnDraw();

	OnPostDraw();

	if( m_pDrawListener )
		m_pDrawListener->Draw3D( GetClientSize() );
}

void CwxBase3DView::ApplyCameraToScene()
{
	if( !m_pCamera )
		return;

	const auto& vecOrigin = m_pCamera->GetOrigin();
	const auto vecAngles = m_pCamera->GetViewDirection();

	glTranslatef( -vecOrigin[ 0 ], -vecOrigin[ 1 ], -vecOrigin[ 2 ] );

	glRotatef( vecAngles[ 0 ], 1.0f, 0.0f, 0.0f );
	glRotatef( vecAngles[ 1 ], 0.0f, 0.0f, 1.0f );
}

void CwxBase3DView::MouseEvents( wxMouseEvent& event )
{
	if( !m_pCamera )
	{
		event.Skip();
		return;
	}

	if( event.ButtonDown() )
	{
		m_OldCamera.SetOrigin( m_pCamera->GetOrigin() );
		m_OldCamera.SetViewDirection( m_pCamera->GetViewDirection() );
		m_vecOldCoords.x = event.GetX();
		m_vecOldCoords.y = event.GetY();

		m_iButtonsDown |= event.GetButton();
	}
	else if( event.ButtonUp() )
	{
		m_iButtonsDown &= ~event.GetButton();
	}
	else if( event.Dragging() )
	{
		if( event.LeftIsDown() && m_iButtonsDown & wxMOUSE_BTN_LEFT )
		{
			if( event.GetModifiers() & wxMOD_SHIFT )
			{
				m_pCamera->GetOrigin().x = m_OldCamera.GetOrigin().x - ( float ) ( event.GetX() - m_vecOldCoords.x );
				m_pCamera->GetOrigin().y = m_OldCamera.GetOrigin().y + ( float ) ( event.GetY() - m_vecOldCoords.y );
			}
			else
			{
				//TODO: this should be a vector, not an angle
				glm::vec3 vecViewDir = m_OldCamera.GetViewDirection();

				vecViewDir.x += ( float ) ( event.GetY() - m_vecOldCoords.y );
				vecViewDir.y += ( float ) ( event.GetX() - m_vecOldCoords.x );

				m_pCamera->SetViewDirection( vecViewDir );
			}
		}
		else if( event.RightIsDown() && m_iButtonsDown & wxMOUSE_BTN_RIGHT )
		{
			m_pCamera->GetOrigin().z = m_OldCamera.GetOrigin().z + ( float ) ( event.GetY() - m_vecOldCoords.y );
		}

		Refresh();
	}
	else
	{
		event.Skip();
	}
}
}