#include <memory>

#include <wx/image.h>

#include "CSpriteViewer.h"
#include "../settings/CSpriteViewerSettings.h"
#include "../CSpriteViewerState.h"

#include "graphics/GraphicsUtils.h"

#include "engine/shared/renderer/IRenderContext.h"

#include "game/entity/CSpriteEntity.h"
#include "engine/shared/renderer/sprite/ISpriteRenderer.h"
#include "engine/shared/renderer/sprite/CSpriteRenderInfo.h"

#include "ui/wx/CwxOpenGL.h"

#include "C3DView.h"

//TODO: remove
extern sprite::ISpriteRenderer* g_pSpriteRenderer;
extern renderer::IRenderContext* g_pRenderContext;

namespace sprview
{
wxBEGIN_EVENT_TABLE( C3DView, CwxBaseGLCanvas )
wxEND_EVENT_TABLE()

C3DView::C3DView( wxWindow* pParent, CSpriteViewer* const pSpriteViewer, I3DViewListener* pListener )
	: CwxBaseGLCanvas( pParent, wxID_ANY, wxDefaultPosition, wxSize( 600, 400 ) )
	, m_pSpriteViewer( pSpriteViewer )
	, m_pListener( pListener )
{
}

C3DView::~C3DView()
{
	SetCurrent( *GetContext() );

	glDeleteTexture( m_BackgroundTexture );
}

void C3DView::PrepareForLoad()
{
	SetCurrent( *GetContext() );
}

void C3DView::UpdateView()
{
	Refresh();
	Update();
}

void C3DView::DrawScene()
{
	const Color& backgroundColor = m_pSpriteViewer->GetSettings()->GetBackgroundColor();

	g_pRenderContext->ClearColor( backgroundColor.GetRed() / 255.0f, backgroundColor.GetGreen() / 255.0f, backgroundColor.GetBlue() / 255.0f, 1.0 );

	g_pRenderContext->Clear( renderer::ClearBit::COLOR | renderer::ClearBit::DEPTH );

	const wxSize size = GetClientSize();

	g_pRenderContext->Viewport( 0, 0, size.GetWidth(), size.GetHeight() );

	DrawSpriteInfo();

	if( m_pListener )
		m_pListener->Draw3D( size );
}

void C3DView::DrawSpriteInfo()
{
	const wxSize size = GetClientSize();

	g_pRenderContext->MatrixMode( renderer::MatrixMode::PROJECTION );
	g_pRenderContext->LoadIdentity();

	g_pRenderContext->Ortho( 0.0f, ( vec_t ) size.GetWidth(), ( vec_t ) size.GetHeight(), 0.0f, 1.0f, -1.0f );

	g_pRenderContext->MatrixMode( renderer::MatrixMode::MODEL );
	g_pRenderContext->PushMatrix();
	g_pRenderContext->LoadIdentity();

	g_pRenderContext->MatrixMode( renderer::MatrixMode::VIEW );
	g_pRenderContext->PushMatrix();
	g_pRenderContext->LoadIdentity();

	g_pRenderContext->SetCullFace( renderer::CullFace::FRONT );

	if( auto pEntity = m_pSpriteViewer->GetState()->GetEntity() )
	{
		const glm::vec3& vecScale = pEntity->GetScale();

		//g_pSpriteRenderer->DrawSprite2D( size.GetWidth() / 2, size.GetHeight() / 2, pEntity->GetSprite(), 4 );

		sprite::C2DSpriteRenderInfo renderInfo;

		renderInfo.vecPos			= Vector2D( size.GetWidth() / 2, size.GetHeight() / 2 );
		renderInfo.vecScale			= Vector2D( vecScale.x, vecScale.y );
		renderInfo.pSprite			= pEntity->GetSprite();
		renderInfo.flTransparency	= pEntity->GetTransparency();
		renderInfo.flFrame			= pEntity->GetFrame();

		if( m_pSpriteViewer->GetState()->IsTexFormatOverridden() )
		{
			renderInfo.texFormat = m_pSpriteViewer->GetState()->GetTexFormatOverride();
			renderInfo.bOverrideTexFormat = true;
		}

		renderInfo.vecScale *= m_pSpriteViewer->GetState()->GetScale();

		g_pSpriteRenderer->DrawSprite2D( &renderInfo, renderer::DrawFlag::NONE );
	}

	g_pRenderContext->PopMatrix();

	g_pRenderContext->MatrixMode( renderer::MatrixMode::MODEL );
	g_pRenderContext->PopMatrix();
}

void C3DView::DrawSprite()
{
	const wxSize size = GetClientSize();

	//
	// draw background
	//

	if( m_pSpriteViewer->GetState()->showBackground && m_BackgroundTexture != GL_INVALID_TEXTURE_ID )
	{
		graphics::DrawBackground( m_BackgroundTexture );
	}

	graphics::SetProjection( 65.0f, size.GetWidth(), size.GetHeight() );

	g_pRenderContext->MatrixMode( renderer::MatrixMode::MODEL );
	g_pRenderContext->PushMatrix();
	g_pRenderContext->LoadIdentity();

	g_pRenderContext->MatrixMode( renderer::MatrixMode::VIEW );
	g_pRenderContext->PushMatrix();
	g_pRenderContext->LoadIdentity();

	auto pEntity = m_pSpriteViewer->GetState()->GetEntity();

	if( pEntity )
	{
		const glm::vec3& vecScale = pEntity->GetScale();

		//Determine if an odd number of scale values are negative. The cull face has to be changed if so.
		const float flScale = vecScale.x * vecScale.y * vecScale.z;

		g_pRenderContext->SetCullFace( flScale > 0 ? renderer::CullFace::FRONT : renderer::CullFace::BACK );

		pEntity->Draw( renderer::DrawFlag::NONE );
	}

	g_pRenderContext->PopMatrix();

	g_pRenderContext->MatrixMode( renderer::MatrixMode::MODEL );

	g_pRenderContext->PopMatrix();
}

bool C3DView::LoadBackgroundTexture( const wxString& szFilename )
{
	UnloadBackgroundTexture();

	m_BackgroundTexture = wxOpenGL().glLoadImage( szFilename.c_str() );

	//TODO: notify UI
	m_pSpriteViewer->GetState()->showBackground = m_BackgroundTexture != GL_INVALID_TEXTURE_ID;

	return m_BackgroundTexture != GL_INVALID_TEXTURE_ID;
}

void C3DView::UnloadBackgroundTexture()
{
	glDeleteTexture( m_BackgroundTexture );
}

void C3DView::TakeScreenshot()
{
	SetCurrent( *GetContext() );

	const wxSize size = GetClientSize();

	std::unique_ptr<byte[]> rgbData = std::make_unique<byte[]>( size.GetWidth() * size.GetHeight() * 3 );

	const renderer::ReadBuffer oldReadBuffer = g_pRenderContext->GetReadBuffer();

	//Read currently displayed buffer.
	g_pRenderContext->SetReadBuffer( renderer::ReadBuffer::FRONT );

	//Grab the image from the 3D view itself.
	g_pRenderContext->ReadPixels( 0, 0, size.GetWidth(), size.GetHeight(), renderer::ImageFormat::RGB, rgbData.get() );

	g_pRenderContext->SetReadBuffer( oldReadBuffer );

	//Now ask for a filename.
	wxFileDialog dlg( this );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szFilename = dlg.GetPath();

	//We have to flip the image vertically, since OpenGL reads it upside down.
	graphics::FlipImageVertically( size.GetWidth(), size.GetHeight(), rgbData.get() );

	wxImage image( size.GetWidth(), size.GetHeight(), rgbData.get(), true );

	//TODO: set default extension to bmp if none is given. Also do this for HLMV.
	if( !image.SaveFile( szFilename, wxBITMAP_TYPE_BMP ) )
	{
		wxMessageBox( wxString::Format( "Failed to save image \"%s\"!", szFilename.c_str() ) );
	}
}
}