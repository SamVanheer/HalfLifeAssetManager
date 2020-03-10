#include <memory>

#include "shared/Logging.h"

#include "graphics/GLRenderTarget.h"

#include "engine/shared/renderer/IRenderContext.h"

#include "CwxOpenGL.h"

//TODO: remove.
extern renderer::IRenderContext* g_pRenderContext;

CwxOpenGL* CwxOpenGL::m_pInstance = nullptr;

CwxOpenGL& CwxOpenGL::CreateInstance()
{
	if( m_pInstance )
	{
		Warning( "CwxOpenGL::CreateInstance called multiple times!\n" );
		return *m_pInstance;
	}

	m_pInstance = new CwxOpenGL();

	return *m_pInstance;
}

void CwxOpenGL::DestroyInstance()
{
	if( m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

CwxOpenGL& CwxOpenGL::GetInstance()
{
	return *m_pInstance;
}

bool CwxOpenGL::InstanceExists()
{
	return m_pInstance != nullptr;
}

CwxOpenGL::CwxOpenGL()
{
}

CwxOpenGL::~CwxOpenGL()
{
}

/*
*	Must be called on startup to initialize all needed settings.
*	pContextAttrs:		If set, specifies context attributes. A copy is kept, the passed in parameter is not stored.
*	pCanvasAttributes:	If set, specifies canvas attributes. A copy is kept, the passed in parameter is not stored.
*	Returns true if initialization was successful.
*/
bool CwxOpenGL::Initialize( const wxGLAttributes& canvasAttributes, const wxGLContextAttrs* const pContextAttrs )
{
	m_CanvasAttributes = canvasAttributes;

	if( pContextAttrs )
	{
		m_bContextAttributesSet = true;
		m_ContextAttributes = *pContextAttrs;
	}

	return true;
}

/*
*	Shuts down all OpenGL data tracking. Must be called on application shutdown.
*/
void CwxOpenGL::Shutdown()
{
	//TODO: this requires the gl context to be current. It requires a canvas to do so, meaning this has to happen before the last canvas is destroyed.
	if( m_pScratchTarget )
	{
		delete m_pScratchTarget;
		m_pScratchTarget = nullptr;
	}

	if( m_pContext )
	{
		delete m_pContext;
		m_pContext = nullptr;
	}

	if( m_bContextAttributesSet )
	{
		m_bContextAttributesSet = false;
		m_ContextAttributes.Reset();
	}

	m_CanvasAttributes.Reset();
}

/*
*	Gets a context that can be used with the given canvas.
*	All canvases created in a program must use identical settings.
*/
wxGLContext* CwxOpenGL::GetContext( wxGLCanvas* pCanvas )
{
	wxASSERT( pCanvas != nullptr );

	if( !m_pContext )
	{
		m_pContext = new wxGLContext( pCanvas, nullptr, GetContextAttributes() );

		if( !m_pContext->IsOK() )
		{
			delete m_pContext;
			m_pContext = nullptr;

			Error( "Error creating OpenGL context: attributes unsupported by this hardware and/or operating system\n" );

			wxMessageBox( "Error creating OpenGL context: attributes unsupported by this hardware and/or operating system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );

			//TODO: exit more gracefully.
			wxExit();

			return nullptr;
		}

		//Initalize GLEW if needed.
		if( !IsPostInitialized() )
		{
			pCanvas->SetCurrent( *m_pContext );

			if( !PostInitialize() )
			{
				//TODO: exit more gracefully.
				wxExit();

				return nullptr;
			}
		}
	}

	return m_pContext;
}

GLRenderTarget* CwxOpenGL::GetScratchTarget()
{
	if( m_pScratchTarget )
		return m_pScratchTarget;

	//Need a context before we can create this.
	if( !m_pContext )
	{
		wxASSERT_MSG( m_pContext, "The scratch target requires a context to be created!" );
		return nullptr;
	}

	//If framebuffer creation is not supported, always report failure
	if( !glGenFramebuffers )
	{
		return nullptr;
	}

	m_pScratchTarget = new GLRenderTarget( true );

	return m_pScratchTarget;
}

GLuint CwxOpenGL::glLoadImage( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return GL_INVALID_TEXTURE_ID;

	if( !wxFileExists( pszFilename ) )
	{
		wxMessageBox( wxString::Format( "File \"%s\" does not exist\n", pszFilename ) );
		return GL_INVALID_TEXTURE_ID;
	}

	wxImage image( pszFilename );

	if( !image.IsOk() )
	{
		wxMessageBox( wxString::Format( "An error occurred while loading \"%s\"\n", pszFilename ) );
		return GL_INVALID_TEXTURE_ID;
	}

	const unsigned char* const pData = image.GetData();
	const unsigned char* const pAlpha = image.GetAlpha();

	const int BPP = image.HasAlpha() ? 4 : 3;

	std::unique_ptr<GLubyte[]> pImageData( new GLubyte[ image.GetWidth() * image.GetHeight() * BPP ] );

	for( int y = 0; y < image.GetHeight(); ++y )
	{
		for( int x = 0; x < image.GetWidth(); ++x )
		{
			for( int i = 0; i < 3; ++i )
			{
				pImageData[ ( x + y * image.GetWidth() ) * BPP + i ] = pData[ ( x + ( y ) * image.GetWidth() ) * 3 + i ];
			}

			if( image.HasAlpha() )
			{
				pImageData[ ( x + y * image.GetWidth() ) * BPP + 3 ] = pAlpha[ x + ( y ) * image.GetWidth() ];
			}
		}
	}

	const renderer::ImageFormat format = image.HasAlpha() ? renderer::ImageFormat::RGBA : renderer::ImageFormat::RGB;

	renderer::HTexture_t tex = g_pRenderContext->CreateTexture( 0, format, image.GetWidth(), image.GetHeight(), pImageData.get() );

	g_pRenderContext->BindTexture( tex );

	g_pRenderContext->SetMinMagFilters( renderer::MinFilter::LINEAR, renderer::MagFilter::LINEAR );

	//TODO: update all uses to use HTexture_t
	return ( GLuint ) tex;
}