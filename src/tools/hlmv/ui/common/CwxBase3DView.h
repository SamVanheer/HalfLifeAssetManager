#ifndef UI_WX_SHARED_CWXBASE3DVIEW_H
#define UI_WX_SHARED_CWXBASE3DVIEW_H

#include <glm/vec2.hpp>

#include "CwxBaseGLCanvas.h"

namespace ui
{
/**
*	Base class for 3D views. Provides movement support and screenshot functionality.
*	Abstract.
*/
class CwxBase3DView : public ui::CwxBaseGLCanvas
{
protected:
	CwxBase3DView( wxWindow* pParent, wxWindowID id = wxID_ANY,
				   const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
				   long style = 0,
				   const wxString& name = wxGLCanvasName,
				   const wxPalette& palette = wxNullPalette );

public:
	virtual ~CwxBase3DView() = 0;

protected:
	wxDECLARE_EVENT_TABLE();

	void DrawScene() override final;

	virtual void OnPreDraw() {}
	virtual void OnDraw() {}
	virtual void OnPostDraw() {}
};
}

#endif //UI_WX_SHARED_CWXBASE3DVIEW_H