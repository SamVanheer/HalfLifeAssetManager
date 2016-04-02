#ifndef C3DVIEW_H
#define C3DVIEW_H

#include "wxInclude.h"

#include <wx/glcanvas.h>

#include "model/utility/OpenGL.h"

#include "mathlib.h"

#include "model/options/COptions.h"

class I3DViewListener
{
public:
	virtual ~I3DViewListener() = 0;

	virtual void Draw3D( const wxSize& size ) = 0;
};

inline I3DViewListener::~I3DViewListener()
{
}

class C3DView final : public wxGLCanvas
{
public:
	C3DView( wxWindow* pParent, I3DViewListener* pListener = nullptr );
	~C3DView();

	void Paint( wxPaintEvent& event );

	void MouseEvents( wxMouseEvent& event );

	//Tells the 3D view to prepare for model loading.
	void PrepareForLoad();

	void UpdateView();

protected:
	wxDECLARE_EVENT_TABLE();

private:

	void DrawFloor();
	void SetupRenderMode( RenderMode renderMode = RenderMode::INVALID );

private:
	I3DViewListener* m_pListener;

	wxGLContext* m_pContext;

	GLuint m_BackgroundTexture	= GL_INVALID_TEXTURE_ID;
	GLuint m_GroundTexture		= GL_INVALID_TEXTURE_ID;

	float m_flOldRotX = 0;
	float m_flOldRotY = 0;

	vec3_t m_vecOldTrans = { 0, 0, 0 };

	float m_flOldX = 0;
	float m_flOldY = 0;

	wxLongLong m_iPrevTime = 0;

	//Tracks mouse button state. Used to prevent input from being mistakingly applied (e.g. double click from dialog spilling over as drag).
	int m_iButtonsDown = wxMOUSE_BTN_NONE;

private:
	C3DView( const C3DView& ) = delete;
	C3DView& operator=( const C3DView& ) = delete;
};

#endif //C3DVIEW_H