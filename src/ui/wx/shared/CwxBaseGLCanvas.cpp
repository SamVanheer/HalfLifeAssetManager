#include "ui/wx/CwxOpenGL.h"

#include "CwxBaseGLCanvas.h"

namespace ui
{
wxBEGIN_EVENT_TABLE( CwxBaseGLCanvas, wxGLCanvas )
	EVT_PAINT( CwxBaseGLCanvas::Paint )
wxEND_EVENT_TABLE()

CwxBaseGLCanvas::CwxBaseGLCanvas( wxWindow* pParent, wxWindowID id, const wxPoint& pos, const wxSize& size )
	: wxGLCanvas( pParent, wxOpenGL().GetCanvasAttributes(), id, pos, size )
{
	m_pContext = wxOpenGL().GetContext( this );
}

CwxBaseGLCanvas::~CwxBaseGLCanvas()
{
}

void CwxBaseGLCanvas::Paint( wxPaintEvent& event )
{
	SetCurrent( *m_pContext );

	//Can't use the DC to draw anything since OpenGL draws over it.
	wxPaintDC( this );

	DrawScene();

	glFlush();
	SwapBuffers();
}
}