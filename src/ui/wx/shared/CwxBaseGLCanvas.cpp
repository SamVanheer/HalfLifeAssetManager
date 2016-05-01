#include "ui/wx/CwxOpenGL.h"

#include "CwxBaseGLCanvas.h"

namespace ui
{
wxBEGIN_EVENT_TABLE( CwxBaseGLCanvas, wxGLCanvas )
	EVT_PAINT( CwxBaseGLCanvas::Paint )
wxEND_EVENT_TABLE()

CwxBaseGLCanvas::CwxBaseGLCanvas( wxWindow* pParent, wxWindowID id, 
								  const wxPoint& pos, const wxSize& size, 
								  long style,
								  const wxString& name,
								  const wxPalette& palette )
	: wxGLCanvas( pParent, wxOpenGL().GetCanvasAttributes(), id, pos, size, style, name, palette )
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

	SwapBuffers();

	//Get any errors that were logged during this frame.
	wxOpenGL().GetErrors();
}
}