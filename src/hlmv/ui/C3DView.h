#ifndef C3DVIEW_H
#define C3DVIEW_H

#include "wxHLMV.h"

#include "graphics/OpenGL.h"

//Must be included after OpenGL.h because GLEW replaces gl.h
#include <wx/glcanvas.h>

#include "utility/mathlib.h"

#include "studiomodel/StudioModel.h"

#include "graphics/Constants.h"

namespace hlmv
{
class CHLMVState;
}

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
	static const float FLOOR_SIDE_LENGTH;

public:
	C3DView( wxWindow* pParent, hlmv::CHLMVState* const pSettings, I3DViewListener* pListener = nullptr );
	~C3DView();

	void Paint( wxPaintEvent& event );

	void MouseEvents( wxMouseEvent& event );

	//Tells the 3D view to prepare for model loading.
	void PrepareForLoad();

	void UpdateView();

	bool LoadBackgroundTexture( const wxString& szFilename );
	void UnloadBackgroundTexture();

	bool LoadGroundTexture( const wxString& szFilename );
	void UnloadGroundTexture();

	void SaveUVMap( const wxString& szFilename, const int iTexture );

protected:
	wxDECLARE_EVENT_TABLE();

private:

	void SetupRenderMode( RenderMode renderMode = RenderMode::INVALID );

	void DrawTexture( const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines, const mstudiomesh_t* const pUVMesh );

	void DrawModel();

	void CreateUVFrameBuffer();

	void DestroyUVFrameBuffer();

	void SetUVRenderTargetDimensions( const int iWidth, const int iHeight );

private:
	hlmv::CHLMVState* const m_pSettings;

	I3DViewListener* m_pListener;

	wxGLContext* m_pContext;

	GLuint m_BackgroundTexture	= GL_INVALID_TEXTURE_ID;
	GLuint m_GroundTexture		= GL_INVALID_TEXTURE_ID;

	float m_flOldRotX = 0;
	float m_flOldRotY = 0;

	vec3_t m_vecOldTrans = { 0, 0, 0 };

	float m_flOldX = 0;
	float m_flOldY = 0;

	//Tracks mouse button state. Used to prevent input from being mistakingly applied (e.g. double click from dialog spilling over as drag).
	int m_iButtonsDown = wxMOUSE_BTN_NONE;

	//UV map framebuffer and render target.
	GLuint m_UVFrameBuffer = 0;
	GLuint m_UVRenderTarget = GL_INVALID_TEXTURE_ID;

private:
	C3DView( const C3DView& ) = delete;
	C3DView& operator=( const C3DView& ) = delete;
};

#endif //C3DVIEW_H