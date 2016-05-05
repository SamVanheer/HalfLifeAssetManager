#ifndef UI_WX_SHARED_CWXBASE3DVIEW_H
#define UI_WX_SHARED_CWXBASE3DVIEW_H

#include <glm/vec2.hpp>

#include "CwxBaseGLCanvas.h"

namespace ui
{
class I3DViewListener
{
public:
	virtual ~I3DViewListener() = 0;

	virtual void Draw3D( const wxSize& size ) = 0;
};

inline I3DViewListener::~I3DViewListener()
{
}

/**
*	Base class for 3D views. Provides movement support and screenshot functionality.
*	Abstract.
*/
class CwxBase3DView : public ui::CwxBaseGLCanvas
{
protected:
	CwxBase3DView( wxWindow* pParent, I3DViewListener* pDrawListener = nullptr, wxWindowID id = wxID_ANY,
				   const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
				   long style = 0,
				   const wxString& name = wxGLCanvasName,
				   const wxPalette& palette = wxNullPalette );

public:
	virtual ~CwxBase3DView() = 0;

	/**
	*	Gets the draw listener.
	*	@return Draw listener.
	*/
	I3DViewListener* GetDrawListener() { return m_pDrawListener; }

	/**
	*	Sets the draw listener.
	*	@param pDrawListener Listener to set. May be null.
	*/
	void SetDrawListener( I3DViewListener* pDrawListener )
	{
		m_pDrawListener = pDrawListener;
	}

protected:
	wxDECLARE_EVENT_TABLE();

	void DrawScene() override final;

	virtual void OnPreDraw() {}
	virtual void OnDraw() {}
	virtual void OnPostDraw() {}

private:
	I3DViewListener* m_pDrawListener = nullptr;
};
}

#endif //UI_WX_SHARED_CWXBASE3DVIEW_H