#include <memory>

#include <wx/image.h>

#include "CSpriteViewer.h"
#include "../settings/CSpriteViewerSettings.h"
#include "../CSpriteViewerState.h"

#include "graphics/GraphicsUtils.h"

#include "game/entity/CSpriteEntity.h"
#include "engine/shared/renderer/sprite/ISpriteRenderer.h"

#include "ui/wx/CwxOpenGL.h"

#include "C3DView.h"

//TODO: remove
extern sprite::ISpriteRenderer* g_pSpriteRenderer;

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

	glClearColor( backgroundColor.GetRed() / 255.0f, backgroundColor.GetGreen() / 255.0f, backgroundColor.GetBlue() / 255.0f, 1.0 );

	const wxSize size = GetClientSize();

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glViewport( 0, 0, size.GetX(), size.GetY() );

	DrawSpriteInfo();

	if( m_pListener )
		m_pListener->Draw3D( size );
}

void C3DView::DrawSpriteInfo()
{
	const wxSize size = GetClientSize();

	//TODO: these matrices shouldn't be here.
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	//TODO: get size from window
	glOrtho( 0.0f, ( float ) size.GetWidth(), ( float ) size.GetHeight(), 0.0f, 1.0f, -1.0f );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glCullFace( GL_FRONT );

	if( auto pEntity = m_pSpriteViewer->GetState()->GetEntity() )
	{
		g_pSpriteRenderer->DrawSprite2D( size.GetWidth() / 2, size.GetHeight() / 2, pEntity->GetSprite(), 4 );
	}

	glPopMatrix();
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

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	auto pEntity = m_pSpriteViewer->GetState()->GetEntity();

	if( pEntity )
	{
		const glm::vec3& vecScale = pEntity->GetScale();

		//Determine if an odd number of scale values are negative. The cull face has to be changed if so.
		const float flScale = vecScale.x * vecScale.y * vecScale.z;

		glCullFace( flScale > 0 ? GL_FRONT : GL_BACK );

		pEntity->Draw( renderer::DrawFlag::NONE );
	}

	glPopMatrix();
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
	graphics::FlipImageVertically( size.GetWidth(), size.GetHeight(), rgbData.get() );

	wxImage image( size.GetWidth(), size.GetHeight(), rgbData.get(), true );

	if( !image.SaveFile( szFilename, wxBITMAP_TYPE_BMP ) )
	{
		wxMessageBox( wxString::Format( "Failed to save image \"%s\"!", szFilename.c_str() ) );
	}
}
}