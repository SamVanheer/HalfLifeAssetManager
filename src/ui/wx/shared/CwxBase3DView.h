#ifndef UI_WX_SHARED_CWXBASE3DVIEW_H
#define UI_WX_SHARED_CWXBASE3DVIEW_H

#include <glm/vec2.hpp>

#include "graphics/CCamera.h"

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

	/**
	*	Gets the current camera.
	*/
	const graphics::CCamera* GetCamera() const { return m_pCamera; }

	/**
	*	@copydoc GetCamera() const
	*/
	graphics::CCamera* GetCamera() { return m_pCamera; }

	/**
	*	Sets the current camera.
	*/
	void SetCamera( graphics::CCamera* pCamera );

protected:
	wxDECLARE_EVENT_TABLE();

	void DrawScene() override final;

	virtual void OnPreDraw() {}
	virtual void OnDraw() {}
	virtual void OnPostDraw() {}

	/**
	*	Applies the current camera settings to the scene.
	*/
	void ApplyCameraToScene();

	virtual void MouseEvents( wxMouseEvent& event );

private:
	I3DViewListener* m_pDrawListener = nullptr;

	graphics::CCamera* m_pCamera = nullptr;

	//Used for rotation and translation.
	graphics::CCamera m_OldCamera;

	//Old mouse coordinates.
	glm::vec2 m_vecOldCoords;

	//Tracks mouse button state. Used to prevent input from being mistakingly applied (e.g. prevent double click from dialog spilling over as drag).
	int m_iButtonsDown = wxMOUSE_BTN_NONE;
};
}

#endif //UI_WX_SHARED_CWXBASE3DVIEW_H