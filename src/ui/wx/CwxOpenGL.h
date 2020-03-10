#ifndef UI_CWXOPENGL_H
#define UI_CWXOPENGL_H

#include "ui/wxHLMV.h"

#include "graphics/OpenGL.h"

#include <wx/glcanvas.h>

class GLRenderTarget;

/*
*	Singleton class that stores all OpenGL data used by any program.
*/
class CwxOpenGL final : protected CBaseOpenGL
{
public:
	static CwxOpenGL& CreateInstance();
	static void DestroyInstance();
	static bool InstanceExists();
	static CwxOpenGL& GetInstance();

	bool Initialize( const wxGLAttributes& canvasAttributes, const wxGLContextAttrs* const pContextAttrs = nullptr );
	void Shutdown();

	const wxGLAttributes& GetCanvasAttributes() const { return m_CanvasAttributes; }

	const wxGLContextAttrs* GetContextAttributes() const { return m_bContextAttributesSet ? &m_ContextAttributes : nullptr; }

	/**
	*	Gets the context, if it exists.
	*/
	wxGLContext* GetContext() { return m_pContext; }

	/**
	*	Gets a context for use by a canvas. The context is created if it doesn't exist yet.
	*	@param pCanvas Canvas used to create the context.
	*/
	wxGLContext* GetContext( wxGLCanvas* pCanvas );

	GLRenderTarget* GetScratchTarget();

	using CBaseOpenGL::GetErrors;

	GLuint glLoadImage( const char* const pszFilename ) override final;

private:
	CwxOpenGL();
	~CwxOpenGL();

private:
	static CwxOpenGL* m_pInstance;

	wxGLAttributes		m_CanvasAttributes;
	wxGLContextAttrs	m_ContextAttributes;
	bool				m_bContextAttributesSet = false;	//Whether any context attributes have been set.

	wxGLContext*		m_pContext = nullptr;				//The context used by all windows.

	GLRenderTarget*		m_pScratchTarget = nullptr;			//Render target used for one off drawing and conversion operations.

private:
	CwxOpenGL( const CwxOpenGL& ) = delete;
	CwxOpenGL& operator=( const CwxOpenGL& ) = delete;
};

inline CwxOpenGL& wxOpenGL()
{
	return CwxOpenGL::GetInstance();
}

#endif //UI_CWXOPENGL_H