#include <memory>

#include <wx/image.h>

#include "CHLMV.h"
#include "../settings/CHLMVSettings.h"
#include "../CHLMVState.h"

#include "graphics/GraphicsHelpers.h"
#include "graphics/GLRenderTarget.h"

#include "shared/studiomodel/CStudioModelRenderer.h"

#include "game/entity/CStudioModelEntity.h"

#include "ui/wx/CwxOpenGL.h"

#include "C3DView.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( C3DView, CwxBase3DView )
	//EVT_MOUSE_EVENTS( C3DView::MouseEvents )
wxEND_EVENT_TABLE()

C3DView::C3DView( wxWindow* pParent, CHLMV* const pHLMV, I3DViewListener* pListener )
	: CwxBase3DView( pParent, nullptr, wxID_ANY, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pHLMV( pHLMV )
	, m_pListener( pListener )
{
	SetCamera( &m_pHLMV->GetState()->camera );
}

C3DView::~C3DView()
{
	SetCurrent( *GetContext() );

	glDeleteTexture( m_GroundTexture );
	glDeleteTexture( m_BackgroundTexture );
}

void C3DView::MouseEvents( wxMouseEvent& event )
{
	//Ignore input in weapon origin mode.
	//TODO: refactor
	if( m_pHLMV->GetState()->useWeaponOrigin || m_pHLMV->GetState()->showTexture )
	{
		event.Skip();
		return;
	}

	CwxBase3DView::MouseEvents( event );
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

void C3DView::SetupRenderMode( RenderMode renderMode )
{
	if( renderMode == RenderMode::INVALID )
		renderMode = m_pHLMV->GetState()->renderMode;

	graphics::helpers::SetupRenderMode( renderMode );
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

	graphics::helpers::SetProjection( size.GetWidth(), size.GetHeight() );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	if( m_pHLMV->GetState()->useWeaponOrigin )
	{
		glTranslatef( -m_pHLMV->GetState()->weaponOrigin[ 0 ], -m_pHLMV->GetState()->weaponOrigin[ 1 ], -m_pHLMV->GetState()->weaponOrigin[ 2 ] );

		glRotatef( -90, 1.0f, 0.0f, 0.0f );
		glRotatef( 90, 0.0f, 0.0f, 1.0f );
	}
	else
	{
		ApplyCameraToScene();
	}

	glm::vec3 vecViewerRight = studiomodel::renderer().GetViewerRight();

	vecViewerRight[ 0 ] = vecViewerRight[ 1 ] = m_pHLMV->GetState()->camera.GetOrigin()[ 2 ];

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
												  m_pHLMV->GetSettings()->GetFloorLength() );
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