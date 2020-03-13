#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <wx/image.h>
#include <wx/notebook.h>

#include "CModelViewerApp.h"
#include "../settings/CHLMVSettings.h"
#include "../CHLMVState.h"

#include "MouseOpFlag.h"
#include "controlpanels/CBaseControlPanel.h"

#include "graphics/GraphicsUtils.h"
#include "graphics/GraphicsHelpers.h"

#include "shared/renderer/studiomodel/IStudioModelRenderer.h"

#include "game/entity/CStudioModelEntity.h"

#include "wx/CwxOpenGL.h"

#include "CMainPanel.h"

#include "controlpanels/CTexturesPanel.h"

#include "C3DView.h"

//TODO: remove
extern studiomdl::IStudioModelRenderer* g_pStudioMdlRenderer;

namespace hlmv
{
wxBEGIN_EVENT_TABLE( C3DView, CwxBase3DView )
	EVT_MOUSE_EVENTS( C3DView::MouseEvents )
wxEND_EVENT_TABLE()

C3DView::C3DView( wxWindow* pParent, CModelViewerApp* const pHLMV, CMainPanel* const pMainPanel )
	: CwxBase3DView( pParent, wxID_ANY, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pHLMV( pHLMV )
	, m_pMainPanel( pMainPanel )
{
	wxASSERT( pMainPanel );
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

	const int centerX = size.GetX() / 2;
	const int centerY = size.GetY() / 2;

	if (m_pHLMV->GetState()->drawCrosshair)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0.0f, (float) size.GetX(), (float) size.GetY(), 0.0f, 1.0f, -1.0f);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_CULL_FACE);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDisable(GL_TEXTURE_2D);

		const Color& crosshairColor = m_pHLMV->GetSettings()->GetCrosshairColor();

		glColor4f(crosshairColor.GetRed() / 255.0f, crosshairColor.GetGreen() / 255.0f, crosshairColor.GetBlue() / 255.0f, 1.0);

		glPointSize(CROSSHAIR_LINE_WIDTH);
		glLineWidth(CROSSHAIR_LINE_WIDTH);

		glBegin(GL_POINTS);

		glVertex2f(centerX - CROSSHAIR_LINE_WIDTH / 2, centerY + 1);

		glEnd();

		glBegin(GL_LINES);

		glVertex2f(centerX - CROSSHAIR_LINE_START, centerY);
		glVertex2f(centerX - CROSSHAIR_LINE_END, centerY);

		glVertex2f(centerX + CROSSHAIR_LINE_START, centerY);
		glVertex2f(centerX + CROSSHAIR_LINE_END, centerY);

		glVertex2f(centerX, centerY - CROSSHAIR_LINE_START);
		glVertex2f(centerX, centerY - CROSSHAIR_LINE_END);

		glVertex2f(centerX, centerY + CROSSHAIR_LINE_START);
		glVertex2f(centerX, centerY + CROSSHAIR_LINE_END);

		glEnd();

		glPointSize(1);
		glLineWidth(1);

		glPopMatrix();
	}

	if (m_pHLMV->GetState()->drawGuidelines)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0.0f, (float) size.GetX(), (float) size.GetY(), 0.0f, 1.0f, -1.0f);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_CULL_FACE);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDisable(GL_TEXTURE_2D);

		const Color& crosshairColor = m_pHLMV->GetSettings()->GetCrosshairColor();

		glColor4f(crosshairColor.GetRed() / 255.0f, crosshairColor.GetGreen() / 255.0f, crosshairColor.GetBlue() / 255.0f, 1.0);

		glPointSize(GUIDELINES_LINE_WIDTH);
		glLineWidth(GUIDELINES_LINE_WIDTH);

		glBegin(GL_POINTS);

		for (int yPos = size.GetY() - GUIDELINES_LINE_LENGTH; yPos >= centerY + CROSSHAIR_LINE_END; yPos -= GUIDELINES_OFFSET)
		{
			glVertex2f(centerX - GUIDELINES_LINE_WIDTH, yPos);
		}

		glEnd();

		glBegin(GL_LINES);

		for (int yPos = size.GetY() - GUIDELINES_LINE_LENGTH - GUIDELINES_POINT_LINE_OFFSET - GUIDELINES_LINE_WIDTH;
			yPos >= centerY + CROSSHAIR_LINE_END + GUIDELINES_LINE_LENGTH;
			yPos -= GUIDELINES_OFFSET)
		{
			glVertex2f(centerX, yPos);
			glVertex2f(centerX, yPos - GUIDELINES_LINE_LENGTH);
		}

		glEnd();

		const float flWidth = size.GetY() * (16 / 9.0);

		glLineWidth(GUIDELINES_EDGE_WIDTH);

		glBegin(GL_LINES);

		glVertex2f((size.GetX() / 2) - (flWidth / 2), 0);
		glVertex2f((size.GetX() / 2) - (flWidth / 2), size.GetY());

		glVertex2f((size.GetX() / 2) + (flWidth / 2), 0);
		glVertex2f((size.GetX() / 2) + (flWidth / 2), size.GetY());

		glEnd();

		glPointSize(1);
		glLineWidth(1);

		glPopMatrix();
	}
}

void C3DView::ApplyCameraToScene()
{
	auto pCamera = m_pHLMV->GetState()->GetCurrentCamera();

	const auto& vecOrigin = pCamera->GetOrigin();
	const auto vecAngles = pCamera->GetViewDirection();

	const glm::mat4x4 identity = Mat4x4ModelView();

	auto mat = Mat4x4ModelView();

	mat *= glm::translate( -vecOrigin );

	mat *= glm::rotate( glm::radians( vecAngles[ 2 ] ), glm::vec3{ 1, 0, 0 } );

	mat *= glm::rotate( glm::radians( vecAngles[ 0 ] ), glm::vec3{ 0, 1, 0 } );

	mat *= glm::rotate( glm::radians( vecAngles[ 1 ] ), glm::vec3{ 0, 0, 1 } );

	glLoadMatrixf( glm::value_ptr( mat ) );
}

void C3DView::MouseEvents( wxMouseEvent& event )
{
	//Default to no operations if we couldn't find the page.
	MouseOpFlags_t flags = MOUSEOPF_NONE;

	auto pControlPanels = m_pMainPanel->GetControlPanels();

	const int iPage = pControlPanels->GetSelection();

	if( iPage != wxNOT_FOUND )
	{
		flags = static_cast<CBaseControlPanel*>( pControlPanels->GetPage( iPage ) )->GetAllowedMouseOperations();
	}

	//Disable translation and rotation when weapon origin view mode is enabled.
	if( m_pHLMV->GetState()->UsingWeaponOrigin() )
	{
		flags &= ~( MOUSEOPF_TRANSLATE | MOUSEOPF_ROTATE );
	}

	//Always handle button down and up events so state isn't invalid in some edge cases.

	auto pCamera = m_pHLMV->GetState()->GetCurrentCamera();

	const bool bWasTexPanelData = m_bTexPanelMouseData;

	m_bTexPanelMouseData = pControlPanels->GetCurrentPage() == m_pMainPanel->GetTexturesPanel();

	if( event.ButtonDown() )
	{
		if( !m_bTexPanelMouseData )
		{
			m_OldCamera.SetOrigin( pCamera->GetOrigin() );
			m_OldCamera.SetViewDirection( pCamera->GetViewDirection() );
		}
		else
		{
			auto pState = m_pHLMV->GetState();

			pState->iOldTextureXOffset = pState->iTextureXOffset;
			pState->iOldTextureYOffset = pState->iTextureYOffset;

			m_flOldTextureScale = pState->textureScale;
		}

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
		//Reset data if the panel changed.
		if( bWasTexPanelData != m_bTexPanelMouseData )
		{
			m_vecOldCoords.x = event.GetX();
			m_vecOldCoords.y = event.GetY();
		}

		if( !m_bTexPanelMouseData )
		{
			if( event.LeftIsDown() && m_iButtonsDown & wxMOUSE_BTN_LEFT )
			{
				if( event.GetModifiers() & wxMOD_CONTROL )
				{
					if( flags & MOUSEOPF_LIGHTVECTOR )
					{
						glm::vec3 vecLightDir = g_pStudioMdlRenderer->GetLightVector();

						const float DELTA = 0.05f;

						if( event.GetModifiers() & wxMOD_SHIFT )
						{
							if( m_vecOldCoords.x <= event.GetX() )
							{
								vecLightDir.z += DELTA;
							}
							else
							{
								vecLightDir.z -= DELTA;
							}

							m_vecOldCoords.x = event.GetX();

							vecLightDir.z = clamp( vecLightDir.z, -1.0f, 1.0f );
						}
						else
						{
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

							vecLightDir.x = clamp( vecLightDir.x, -1.0f, 1.0f );
							vecLightDir.y = clamp( vecLightDir.y, -1.0f, 1.0f );
						}

						g_pStudioMdlRenderer->SetLightVector( vecLightDir );
					}
				}
				else if( event.GetModifiers() & wxMOD_SHIFT )
				{
					if( flags & MOUSEOPF_TRANSLATE )
					{
						pCamera->GetOrigin().z = m_OldCamera.GetOrigin().z - ( float ) ( event.GetX() - m_vecOldCoords.x );
						pCamera->GetOrigin().x = m_OldCamera.GetOrigin().x + ( float ) ( event.GetY() - m_vecOldCoords.y );
					}
				}
				else
				{
					if( flags & MOUSEOPF_ROTATE )
					{
						auto settings = m_pHLMV->GetSettings();

						//TODO: this should be a vector, not an angle
						glm::vec3 vecViewDir = m_OldCamera.GetViewDirection();

						auto horizontalAdjust = (float) (event.GetX() - m_vecOldCoords.x);
						auto verticalAdjust = (float) (event.GetY() - m_vecOldCoords.y);

						if (settings->InvertHorizontalDraggingDirection())
						{
							vecViewDir.y -= horizontalAdjust;
						}
						else
						{
							vecViewDir.y += horizontalAdjust;
						}

						if (settings->InvertVerticalDraggingDirection())
						{
							vecViewDir.x -= verticalAdjust;
						}
						else
						{
							vecViewDir.x += verticalAdjust;
						}

						pCamera->SetViewDirection( vecViewDir );
					}
				}
			}
			else if( event.RightIsDown() && m_iButtonsDown & wxMOUSE_BTN_RIGHT )
			{
				if( flags & MOUSEOPF_TRANSLATE )
				{
					pCamera->GetOrigin().y = m_OldCamera.GetOrigin().y + ( float ) ( event.GetY() - m_vecOldCoords.y );
				}
			}
		}
		else
		{
			if( event.LeftIsDown() && m_iButtonsDown & wxMOUSE_BTN_LEFT )
			{
				auto pState = m_pHLMV->GetState();

				pState->iTextureXOffset = pState->iOldTextureXOffset + ( float ) ( event.GetX() - m_vecOldCoords.x );
				pState->iTextureYOffset = pState->iOldTextureYOffset + ( float ) ( event.GetY() - m_vecOldCoords.y );
			}
			else if( event.RightIsDown() && m_iButtonsDown & wxMOUSE_BTN_RIGHT )
			{
				const float flDiff = ( event.GetY() - m_vecOldCoords.y ) / -20.0f;

				m_pMainPanel->GetTexturesPanel()->SetScale( static_cast<int>( m_flOldTextureScale + flDiff ) );
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

	DrawTexture( m_pHLMV->GetState()->iTextureXOffset, m_pHLMV->GetState()->iTextureYOffset, size.GetWidth(), size.GetHeight(), 
				 pEntity, iTexture, flTextureScale, bShowUVMap, bOverlayUVMap, bAntiAliasLines, pUVMesh );
}

void C3DView::DrawTexture( const int iXOffset, const int iYOffset, const int iWidth, const int iHeight,
				  CStudioModelEntity* pEntity,
				  const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines,
				  const mstudiomesh_t* const pUVMesh )
{
	assert( pEntity );

	auto pModel = pEntity->GetModel();

	assert( pModel );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho( 0.0f, ( float ) iWidth, ( float ) iHeight, 0.0f, 1.0f, -1.0f );

	const studiohdr_t* const hdr = pModel->GetTextureHeader();

	if( hdr )
	{
		mstudiotexture_t *ptextures = ( mstudiotexture_t * ) ( ( byte * ) hdr + hdr->textureindex );

		const mstudiotexture_t& texture = ptextures[ iTexture ];

		float w = ( float ) texture.width * flTextureScale;
		float h = ( float ) texture.height * flTextureScale;

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();

		glDisable( GL_CULL_FACE );
		glDisable( GL_BLEND );

		if( texture.flags & STUDIO_NF_MASKED )
		{
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, 0.5f );
		}

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		float x = ( ( ( float ) iWidth - w ) / 2 ) + iXOffset;
		float y = ( ( ( float ) iHeight - h ) / 2 ) + iYOffset;

		glDisable( GL_DEPTH_TEST );

		if( bShowUVMap && !bOverlayUVMap )
		{
			glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
			glDisable( GL_TEXTURE_2D );
			glRectf( x, y, x + w, y + h );
		}

		if( !bShowUVMap || bOverlayUVMap )
		{
			glEnable( GL_TEXTURE_2D );
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			glBindTexture( GL_TEXTURE_2D, pModel->GetTextureId( iTexture ) );

			glBegin( GL_TRIANGLE_STRIP );

			glTexCoord2f( 0, 0 );
			glVertex2f( x, y );

			glTexCoord2f( 1, 0 );
			glVertex2f( x + w, y );

			glTexCoord2f( 0, 1 );
			glVertex2f( x, y + h );

			glTexCoord2f( 1, 1 );
			glVertex2f( x + w, y + h );

			glEnd();

			glBindTexture( GL_TEXTURE_2D, 0 );
		}

		if( bShowUVMap )
		{
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

			CStudioModelEntity::MeshList_t meshes;

			if( pUVMesh )
			{
				meshes.push_back( pUVMesh );
			}
			else
			{
				meshes = pEntity->ComputeMeshList( iTexture );
			}

			graphics::helpers::SetupRenderMode( RenderMode::WIREFRAME, true );

			if( bAntiAliasLines )
			{
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glEnable( GL_LINE_SMOOTH );
			}

			int i;

			const mstudiomesh_t* const* ppMeshes = meshes.data();

			for( size_t uiIndex = 0; uiIndex < meshes.size(); ++uiIndex, ++ppMeshes )
			{
				const short* ptricmds = ( short* ) ( ( byte* ) pModel->GetStudioHeader() + ( *ppMeshes )->triindex );

				while( i = *( ptricmds++ ) )
				{
					if( i < 0 )
					{
						glBegin( GL_TRIANGLE_FAN );
						i = -i;
					}
					else
					{
						glBegin( GL_TRIANGLE_STRIP );
					}

					for( ; i > 0; i--, ptricmds += 4 )
					{
						// FIX: put these in as integer coords, not floats
						glVertex2f( x + ptricmds[ 2 ] * flTextureScale, y + ptricmds[ 3 ] * flTextureScale );
					}
					glEnd();
				}
			}

			if( bAntiAliasLines )
			{
				glDisable( GL_LINE_SMOOTH );
			}
		}

		glPopMatrix();

		glClear( GL_DEPTH_BUFFER_BIT );

		if( texture.flags & STUDIO_NF_MASKED )
			glDisable( GL_ALPHA_TEST );
	}
}

void C3DView::DrawModel()
{
	const wxSize size = GetClientSize();

	//
	// draw background
	//

	if( m_pHLMV->GetState()->showBackground && m_BackgroundTexture != GL_INVALID_TEXTURE_ID && !m_pHLMV->GetState()->showTexture )
	{
		graphics::DrawBackground( m_BackgroundTexture );
	}

	graphics::SetProjection( m_pHLMV->GetState()->GetCurrentFOV(), size.GetWidth(), size.GetHeight() );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	ApplyCameraToScene();

	if( m_pHLMV->GetState()->drawAxes )
	{
		glDisable( GL_TEXTURE_2D );
		glEnable( GL_DEPTH_TEST );

		const float flLength = 50.0f;

		glLineWidth( 1.0f );

		glBegin( GL_LINES );

		glColor3f( 1.0f, 0, 0 );

		glVertex3f( 0, 0, 0 );
		glVertex3f( flLength, 0, 0 );

		glColor3f( 0, 1, 0 );

		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, flLength, 0 );

		glColor3f( 0, 0, 1.0f );

		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, 0, flLength );

		glEnd();
	}

	const auto vecAngles = m_pHLMV->GetState()->GetCurrentCamera()->GetViewDirection();

	auto mat = Mat4x4ModelView();

	mat *= glm::translate( -m_pHLMV->GetState()->GetCurrentCamera()->GetOrigin() );

	mat *= glm::rotate( glm::radians( vecAngles[ 2 ] ), glm::vec3{ 1, 0, 0 } );

	mat *= glm::rotate( glm::radians( vecAngles[ 0 ] ), glm::vec3{ 0, 1, 0 } );

	mat *= glm::rotate( glm::radians( vecAngles[ 1 ] ), glm::vec3{ 0, 0, 1 } );

	const auto vecAbsOrigin = glm::inverse( mat )[ 3 ];
	
	g_pStudioMdlRenderer->SetViewerOrigin( glm::vec3( vecAbsOrigin ) );

	//Originally this was calculated as:
	//vecViewerRight[ 0 ] = vecViewerRight[ 1 ] = vecOrigin[ 2 ];
	//But that vector was incorrect. It mostly affects chrome because of its reflective nature.

	//Grab the angles that the player would have in-game. Since model viewer rotates the world, rather than moving the camera, this has to be adjusted.
	glm::vec3 angViewerDir = -m_pHLMV->GetState()->GetCurrentCamera()->GetViewDirection();

	angViewerDir = angViewerDir + 180.0f;

	glm::vec3 vecViewerRight;

	//We're using the up vector here since the in-game look can only be matched if chrome is rotated.
	AngleVectors( angViewerDir, nullptr, nullptr, &vecViewerRight );

	//Invert it so it points down instead of up. This allows chrome to match the in-game look.
	g_pStudioMdlRenderer->SetViewerRight( -vecViewerRight );

	const unsigned int uiOldPolys = g_pStudioMdlRenderer->GetDrawnPolygonsCount();

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

		renderer::DrawFlags_t flags = renderer::DrawFlag::NONE;

		//Draw wireframe overlay
		if( m_pHLMV->GetState()->wireframeOverlay )
		{
			flags |= renderer::DrawFlag::WIREFRAME_OVERLAY;
		}

		if( m_pHLMV->GetState()->UsingWeaponOrigin() )
		{
			flags |= renderer::DrawFlag::IS_VIEW_MODEL;
		}

		pEntity->Draw( flags );
	}

	//
	// draw ground
	//

	if( m_pHLMV->GetState()->showGround )
	{
		graphics::helpers::DrawFloor( m_pHLMV->GetSettings()->GetFloorLength(), m_GroundTexture, m_pHLMV->GetSettings()->GetGroundColor(), m_pHLMV->GetState()->mirror );
	}

	m_pHLMV->GetState()->drawnPolys = g_pStudioMdlRenderer->GetDrawnPolygonsCount() - uiOldPolys;

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

	//Save off all settings we're about to modify
	//Don't need to restore the viewport since it's reset every frame anyway
	GLint oldReadBuffer;
	GLint oldDrawBuffer;
	GLint oldPackAlignment;

	glGetIntegerv( GL_READ_BUFFER, &oldReadBuffer );
	glGetIntegerv( GL_DRAW_BUFFER, &oldDrawBuffer );
	glGetIntegerv( GL_PACK_ALIGNMENT, &oldPackAlignment );

	//Use the back buffer to avoid modifying displayed data
	glReadBuffer( GL_BACK );
	glDrawBuffer( GL_BACK );

	//Set pack alignment to 1 so no padding is added
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );

	glViewport( 0, 0, texture.width, texture.height );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT );

	DrawTexture( 0, 0, texture.width, texture.height, pEntity, iTexture, 1.0f, true, false, false, m_pHLMV->GetState()->pUVMesh );

	glFlush();
	glFinish();

	std::unique_ptr<byte[]> rgbData = std::make_unique<byte[]>( texture.width * texture.height * 3 );

	glReadPixels( 0, 0, texture.width, texture.height, GL_RGB, GL_UNSIGNED_BYTE, rgbData.get() );

	glReadBuffer( oldReadBuffer );
	glDrawBuffer( oldDrawBuffer );
	glPixelStorei( GL_PACK_ALIGNMENT, oldPackAlignment );

	//We have to flip the image vertically, since OpenGL reads it upside down.
	graphics::FlipImageVertically( texture.width, texture.height, rgbData.get() );

	wxImage image( texture.width, texture.height, rgbData.get(), true );

	if( !image.SaveFile( szFilename, wxBITMAP_TYPE_BMP ) )
	{
		wxMessageBox( wxString::Format( "Failed to save image \"%s\"!", szFilename.c_str() ) );
	}
}

void C3DView::TakeScreenshot()
{
	SetCurrent( *GetContext() );

	//Use the actual viewport size
	GLint viewportSize[ 4 ];

	glGetIntegerv( GL_VIEWPORT, viewportSize );

	const auto width = viewportSize[ 2 ];
	const auto height = viewportSize[ 3 ];

	std::unique_ptr<byte[]> rgbData = std::make_unique<byte[]>( width * height * 3 );

	GLint oldReadBuffer;
	GLint oldPackAlignment;

	glGetIntegerv( GL_READ_BUFFER, &oldReadBuffer );
	glGetIntegerv( GL_PACK_ALIGNMENT, &oldPackAlignment );

	//Read currently displayed buffer.
	glReadBuffer( GL_FRONT );

	//Set pack alignment to 1 so no padding is added
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );

	//Grab the image from the 3D view itself.
	glReadPixels( viewportSize[ 0 ], viewportSize[ 1 ], width, height, GL_RGB, GL_UNSIGNED_BYTE, rgbData.get() );

	glReadBuffer( oldReadBuffer );
	glPixelStorei( GL_PACK_ALIGNMENT, oldPackAlignment );

	//Now ask for a filename.
	wxFileDialog dlg( this, _( "Save screenshot" ), wxEmptyString, "screenshot.png",
		"PNG files (*.png)|*.png|BMP files (*.bmp)|*.bmp|JPG files(*.jpg;*.jpeg)|*.jpg;*.jpeg|All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szFilename = dlg.GetPath();

	//We have to flip the image vertically, since OpenGL reads it upside down.
	graphics::FlipImageVertically( width, height, rgbData.get() );

	wxImage image( width, height, rgbData.get(), true );

	//Let extension determine format
	if( !image.SaveFile( szFilename ) )
	{
		wxMessageBox( wxString::Format( "Failed to save image \"%s\"!", szFilename.c_str() ) );
	}
}
}