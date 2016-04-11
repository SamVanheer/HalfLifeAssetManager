#ifndef UI_CWXOPENGL_H
#define UI_CWXOPENGL_H

#include "wxInclude.h"

#include "graphics/OpenGL.h"

#include <wx/glcanvas.h>

/*
*	Singleton class that stores all OpenGL data used by any program.
*/
class CwxOpenGL final : protected CBaseOpenGL
{
public:
	static CwxOpenGL& GetInstance();

	static void DestroyInstance();

	bool Initialize( const wxGLAttributes& canvasAttributes, const wxGLContextAttrs* const pContextAttrs = nullptr );
	void Shutdown();

	const wxGLAttributes& GetCanvasAttributes() const { return m_CanvasAttributes; }

	const wxGLContextAttrs* GetContextAttributes() const { return m_bContextAttributesSet ? &m_ContextAttributes : nullptr; }

	wxGLContext* GetContext( wxGLCanvas* pCanvas );

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

private:
	CwxOpenGL( const CwxOpenGL& ) = delete;
	CwxOpenGL& operator=( const CwxOpenGL& ) = delete;
};

inline CwxOpenGL& wxOpenGL()
{
	return CwxOpenGL::GetInstance();
}

#endif //UI_CWXOPENGL_H