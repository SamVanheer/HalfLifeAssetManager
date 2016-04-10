#include "model/utility/OpenGL.h"

#include "CwxOpenGL.h"

CwxOpenGL* CwxOpenGL::m_pInstance = nullptr;

CwxOpenGL& CwxOpenGL::GetInstance()
{
	if( !m_pInstance )
	{
		m_pInstance = new CwxOpenGL();
	}

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

		//Initalize GLEW if needed.
		if( !IsOpenGLInitialized() )
		{
			pCanvas->SetCurrent( *m_pContext );

			if( !PostInitializeOpenGL() )
			{
				//TODO: exit more gracefully.
				wxExit();

				return nullptr;
			}
		}
	}

	return m_pContext;
}