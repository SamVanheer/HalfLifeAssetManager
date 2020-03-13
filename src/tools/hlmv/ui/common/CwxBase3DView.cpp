#include "CwxBase3DView.h"

namespace ui
{
wxBEGIN_EVENT_TABLE( CwxBase3DView, CwxBaseGLCanvas )
wxEND_EVENT_TABLE()

CwxBase3DView::CwxBase3DView( wxWindow* pParent, wxWindowID id,
			   const wxPoint& pos, const wxSize& size,
			   long style,
			   const wxString& name,
			   const wxPalette& palette )
	: CwxBaseGLCanvas( pParent, id, pos, size, style, name, palette )
{
}

CwxBase3DView::~CwxBase3DView()
{
}

void CwxBase3DView::DrawScene()
{
	OnPreDraw();

	OnDraw();

	OnPostDraw();
}
}