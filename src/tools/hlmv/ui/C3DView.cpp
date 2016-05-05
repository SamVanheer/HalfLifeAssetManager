#include <memory>

#include <wx/image.h>
#include <wx/notebook.h>

#include "CHLMV.h"
#include "../settings/CHLMVSettings.h"
#include "../CHLMVState.h"

#include "MouseOpFlag.h"
#include "controlpanels/CBaseControlPanel.h"

#include "graphics/GraphicsHelpers.h"
#include "graphics/GLRenderTarget.h"

#include "shared/studiomodel/CStudioModelRenderer.h"

#include "game/entity/CStudioModelEntity.h"

#include "ui/wx/CwxOpenGL.h"

#include "C3DView.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( C3DView, CwxBase3DView )
	EVT_MOUSE_EVENTS( C3DView::MouseEvents )
wxEND_EVENT_TABLE()

C3DView::C3DView( wxWindow* pParent, CHLMV* const pHLMV, wxNotebook* const pControlPanels, I3DViewListener* pListener )
	: CwxBase3DView( pParent, nullptr, wxID_ANY, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pHLMV( pHLMV )
	, m_pControlPanels( pControlPanels )
	, m_pListener( pListener )
{
	wxASSERT( pControlPanels );
}

C3DView::~C3DView()
{
	SetCurrent( *GetContext() );

	glDeleteTexture( m_GroundTexture );
	glDeleteTexture( m_BackgroundTexture );
}

void C3DView::PrepareForLoad()
{
	SetCurrent( *GetContext() );
}

void C3DView::UpdateView()
{
	if( !m_pHLMV->GetState()->pause )
	{
		Refresh();
		Update();
	}
}

void C3DView::OnDraw()
{
	const Color& backgroundColor = m_pHLMV->GetSettings()->GetBackgroundColor();

	glClearColor( backgroundColor.GetRed() / 255.0f, backgroundColor.GetGreen() / 255.0f, backgroundColor.GetBlue() / 255.0f, 1.0 );

	const wxSize size = GetClientSize();

	if( m_pHLMV->GetState()->mirror )
	{
		glClearStencil( 0 );

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	}
	else
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glViewport( 0, 0, size.GetX(), size.GetY() );

	m_pHLMV->GetState()->drawnPolys = 0;

	if( m_pHLMV->GetState()->showTexture )
	{
		DrawTexture( m_pHLMV->GetState()->texture, m_pHLMV->GetState()->textureScale,
					 m_pHLMV->GetState()->showUVMap, m_pHLMV->GetState()->overlayUVMap,
					 m_pHLMV->GetState()->antiAliasUVLines, m_pHLMV->GetState()->pUVMesh );
	}
	else
	{
		DrawModel();
	}

	if( m_pListener )
		m_pListener->Draw3D( size );
}

void C3DView::ApplyCameraToScene()
{
	auto pCamera = m_pHLMV->GetState()->GetCurrentCamera();

	const auto& vecOrigin = pCamera->GetOrigin();
	const auto vecAngles = pCamera->GetViewDirection();

	glTranslatef( -vecOrigin[ 0 ], -vecOrigin[ 1 ], -vecOrigin[ 2 ] );

	glRotatef( vecAngles[ 0 ], 1.0f, 0.0f, 0.0f );
	glRotatef( vecAngles[ 1 ], 0.0f, 0.0f, 1.0f );
}

void C3DView::MouseEvents( wxMouseEvent& event )
{
	//Default to no operations if we couldn't find the page.
	MouseOpFlags_t flags = MOUSEOPF_NONE;

	const int iPage = m_pControlPanels->GetSelection();

	if( iPage != wxNOT_FOUND )
	{
		flags = static_cast<CBaseControlPanel*>( m_pControlPanels->GetPage( iPage ) )->GetAllowedMouseOperations();
	}

	//Disable translation and rotation when weapon origin view mode is enabled.
	if( m_pHLMV->GetState()->UsingWeaponOrigin() )
	{
		flags &= ~( MOUSEOPF_TRANSLATE | MOUSEOPF_ROTATE );
	}

	//Always handle button down and up events so state isn't invalid in some edge cases.

	auto pCamera = m_pHLMV->GetState()->GetCurrentCamera();

	if( event.ButtonDown() )
	{
		m_OldCamera.SetOrigin( pCamera->GetOrigin() );
		m_OldCamera.SetViewDirection( pCamera->GetViewDirection() );
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
				if( flags & MOUSEOPF_TRANSLATE )
				{
					pCamera->GetOrigin().x = m_OldCamera.GetOrigin().x - ( float ) ( event.GetX() - m_vecOldCoords.x );
					pCamera->GetOrigin().y = m_OldCamera.GetOrigin().y + ( float ) ( event.GetY() - m_vecOldCoords.y );
				}
			}
			else if( event.GetModifiers() & wxMOD_CONTROL )
			{
				if( flags & MOUSEOPF_LIGHTVECTOR )
				{
					glm::vec3 vecLightDir = studiomodel::renderer().GetLightVector();

					const float DELTA = 0.05f;

					if( m_vecOldCoords.x <= event.GetX() )
					{
						vecLightDir.x += DELTA;
					}
					else
					{
						vecLightDir.x -= DELTA;
					}

					if( m_vecOldCoords.y <= event.GetY() )
					{
						vecLightDir.y += DELTA;
					}
					else
					{
						vecLightDir.y -= DELTA;
					}

					m_vecOldCoords.x = event.GetX();
					m_vecOldCoords.y = event.GetY();

					vecLightDir.x = clamp( vecLightDir.x, -1.0f, 0.0f );
					vecLightDir.y = clamp( vecLightDir.y, -1.0f, 1.0f );

					studiomodel::renderer().SetLightVector( vecLightDir );
				}
			}
			else
			{
				if( flags & MOUSEOPF_ROTATE )
				{
					//TODO: this should be a vector, not an angle
					glm::vec3 vecViewDir = m_OldCamera.GetViewDirection();

					vecViewDir.x += ( float ) ( event.GetY() - m_vecOldCoords.y );
					vecViewDir.y += ( float ) ( event.GetX() - m_vecOldCoords.x );

					pCamera->SetViewDirection( vecViewDir );
				}
			}
		}
		else if( event.RightIsDown() && m_iButtonsDown & wxMOUSE_BTN_RIGHT )
		{
			if( flags & MOUSEOPF_TRANSLATE )
			{
				pCamera->GetOrigin().z = m_OldCamera.GetOrigin().z + ( float ) ( event.GetY() - m_vecOldCoords.y );
			}
		}

		Refresh();
	}
	else
	{
		event.Skip();
	}
}

void C3DView::SetupRenderMode( RenderMode renderMode )
{
	if( renderMode == RenderMode::INVALID )
		renderMode = m_pHLMV->GetState()->renderMode;

	graphics::helpers::SetupRenderMode( renderMode, m_pHLMV->GetState()->backfaceCulling );
}

void C3DView::DrawTexture( const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines, const mstudiomesh_t* const pUVMesh )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	const wxSize size = GetClientSize();

	graphics::helpers::DrawTexture( size.GetX(), size.GetY(), pEntity, iTexture, flTextureScale, bShowUVMap, bOverlayUVMap, bAntiAliasLines, pUVMesh );
}

void C3DView::DrawModel()
{
	const wxSize size = GetClientSize();

	//
	// draw background
	//

	if( m_pHLMV->GetState()->showBackground && m_BackgroundTexture != GL_INVALID_TEXTURE_ID && !m_pHLMV->GetState()->showTexture )
	{
		graphics::helpers::DrawBackground( m_BackgroundTexture );
	}

	graphics::helpers::SetProjection( m_pHLMV->GetState()->GetCurrentFOV(), size.GetWidth(), size.GetHeight() );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	ApplyCameraToScene();

	studiomodel::renderer().SetViewerOrigin( m_pHLMV->GetState()->GetCurrentCamera()->GetOrigin() );

	//Originally this was calculated as:
	//vecViewerRight[ 0 ] = vecViewerRight[ 1 ] = vecOrigin[ 2 ];
	//But that vector was incorrect. It mostly affects chrome because of its reflective nature.
	const glm::vec3 vecViewerRight = AnglesToVector( m_pHLMV->GetState()->GetCurrentCamera()->GetViewDirection() );

	studiomodel::renderer().SetViewerRight( vecViewerRight );

	const unsigned int uiOldPolys = studiomodel::renderer().GetDrawnPolygonsCount();

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( pEntity )
	{
		// setup stencil buffer and draw mirror
		if( m_pHLMV->GetState()->mirror )
		{
			graphics::helpers::DrawMirroredModel( pEntity, m_pHLMV->GetState()->renderMode,
												  m_pHLMV->GetState()->wireframeOverlay, 
												  m_pHLMV->GetSettings()->GetFloorLength(),
												  m_pHLMV->GetState()->backfaceCulling );
		}
	}

	SetupRenderMode();

	if( pEntity )
	{
		const glm::vec3& vecScale = pEntity->GetScale();

		//Determine if an odd number of scale values are negative. The cull face has to be changed if so.
		const float flScale = vecScale.x * vecScale.y * vecScale.z;

		glCullFace( flScale > 0 ? GL_FRONT : GL_BACK );

		pEntity->Draw( entity::DRAWF_NONE );

		//Draw wireframe overlay
		if( m_pHLMV->GetState()->wireframeOverlay )
		{
			graphics::helpers::DrawWireframeOverlay( pEntity );
		}
	}

	//
	// draw ground
	//

	if( m_pHLMV->GetState()->showGround )
	{
		graphics::helpers::DrawFloor( m_pHLMV->GetSettings()->GetFloorLength(), m_GroundTexture, m_pHLMV->GetSettings()->GetGroundColor(), m_pHLMV->GetState()->mirror );
	}

	m_pHLMV->GetState()->drawnPolys = studiomodel::renderer().GetDrawnPolygonsCount() - uiOldPolys;

	glPopMatrix();
}

bool C3DView::LoadBackgroundTexture( const wxString& szFilename )
{
	UnloadBackgroundTexture();

	m_BackgroundTexture = wxOpenGL().glLoadImage( szFilename.c_str() );

	//TODO: notify UI
	m_pHLMV->GetState()->showBackground = m_BackgroundTexture != GL_INVALID_TEXTURE_ID;

	return m_BackgroundTexture != GL_INVALID_TEXTURE_ID;
}

void C3DView::UnloadBackgroundTexture()
{
	glDeleteTexture( m_BackgroundTexture );
}

bool C3DView::LoadGroundTexture( const wxString& szFilename )
{
	glDeleteTexture( m_GroundTexture );

	m_GroundTexture = wxOpenGL().glLoadImage( szFilename.c_str() );

	return m_GroundTexture != GL_INVALID_TEXTURE_ID;
}

void C3DView::UnloadGroundTexture()
{
	glDeleteTexture( m_GroundTexture );
}

/*
*	Saves the given texture's UV map.
*/
void C3DView::SaveUVMap( const wxString& szFilename, const int iTexture )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	auto pModel = pEntity->GetModel();

	const studiohdr_t* const pHdr = pModel->GetTextureHeader();

	if( !pHdr )
		return;

	const mstudiotexture_t& texture = ( ( mstudiotexture_t* ) ( ( byte* ) pHdr + pHdr->textureindex ) )[ iTexture ];

	SetCurrent( *GetContext() );

	GLRenderTarget* const pScratchTarget = wxOpenGL().GetScratchTarget();

	if( !pScratchTarget )
	{
		wxMessageBox( "Unable to create target to draw UV map to!" );
		return;
	}

	pScratchTarget->Bind();

	pScratchTarget->Setup( texture.width, texture.height, false );

	const GLenum completeness = pScratchTarget->GetStatus();

	if( completeness != GL_FRAMEBUFFER_COMPLETE )
	{
		wxMessageBox( wxString::Format( "UV map framebuffer is incomplete!\n%s (status code %d)", glFrameBufferStatusToString( completeness ), completeness ) );

		pScratchTarget->Unbind();

		return;
	}

	glViewport( 0, 0, texture.width, texture.height );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT );

	graphics::helpers::DrawTexture( texture.width, texture.height, pEntity, iTexture, 1.0f, true, false, false, m_pHLMV->GetState()->pUVMesh );

	pScratchTarget->FinishDraw();

	std::unique_ptr<byte[]> rgbData = std::make_unique<byte[]>( texture.width * texture.height * 3 );

	pScratchTarget->GetPixels( texture.width, texture.height, GL_RGB, GL_UNSIGNED_BYTE, rgbData.get() );

	pScratchTarget->Unbind();

	//We have to flip the image vertically, since OpenGL reads it upside down.
	graphics::helpers::FlipImageVertically( texture.width, texture.height, rgbData.get() );

	wxImage image( texture.width, texture.height, rgbData.get(), true );

	if( !image.SaveFile( szFilename, wxBITMAP_TYPE_BMP ) )
	{
		wxMessageBox( wxString::Format( "Failed to save image \"%s\"!", szFilename.c_str() ) );
	}
}

void C3DView::TakeScreenshot()
{
	SetCurrent( *GetContext() );

	const wxSize size = GetClientSize();

	std::unique_ptr<byte[]> rgbData = std::make_unique<byte[]>( size.GetWidth() * size.GetHeight() * 3 );

	GLint oldReadBuffer;

	glGetIntegerv( GL_READ_BUFFER, &oldReadBuffer );

	//Read currently displayed buffer.
	glReadBuffer( GL_FRONT );

	//Grab the image from the 3D view itself.
	glReadPixels( 0, 0, size.GetWidth(), size.GetHeight(), GL_RGB, GL_UNSIGNED_BYTE, rgbData.get() );

	glReadBuffer( oldReadBuffer );

	//Now ask for a filename.
	wxFileDialog dlg( this );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szFilename = dlg.GetPath();

	//We have to flip the image vertically, since OpenGL reads it upside down.
	graphics::helpers::FlipImageVertically( size.GetWidth(), size.GetHeight(), rgbData.get() );

	wxImage image( size.GetWidth(), size.GetHeight(), rgbData.get(), true );

	if( !image.SaveFile( szFilename, wxBITMAP_TYPE_BMP ) )
	{
		wxMessageBox( wxString::Format( "Failed to save image \"%s\"!", szFilename.c_str() ) );
	}
}
}