#ifndef UI_SHARED_CWXBASEHLCANVAS_H
#define UI_SHARED_CWXBASEHLCANVAS_H

#include "ui/wx/wxInclude.h"

#include "graphics/OpenGL.h"

//Must be included after OpenGL.h because GLEW replaces gl.h
#include <wx/glcanvas.h>

namespace ui
{
/**
*	Base class for GL canvases.
*	Abstract.
*/
class CwxBaseGLCanvas : public wxGLCanvas
{
protected:
	CwxBaseGLCanvas( wxWindow* pParent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
	virtual ~CwxBaseGLCanvas();

	wxGLContext* GetContext() { return m_pContext; }

	/**
	*	Implemented by subclasses: draws the scene.
	*/
	virtual void DrawScene() = 0;

	wxDECLARE_EVENT_TABLE();

	/**
	*	The method that paints the 3D view. Can be overridden by subclasses for completely custom behavior.
	*	Default behavior calls DrawScene.
	*/
	virtual void Paint( wxPaintEvent& event );

private:
	wxGLContext* m_pContext;

private:
	CwxBaseGLCanvas( const CwxBaseGLCanvas& ) = delete;
	CwxBaseGLCanvas& operator=( const CwxBaseGLCanvas& ) = delete;
};
}

#endif //UI_SHARED_CWXBASEHLCANVAS_H
