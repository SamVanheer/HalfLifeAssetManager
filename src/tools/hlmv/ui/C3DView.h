#ifndef C3DVIEW_H
#define C3DVIEW_H

#include "wxHLMV.h"

#include "ui/wx/shared/CwxBaseGLCanvas.h"

#include <glm/vec3.hpp>

#include "graphics/Constants.h"

#include "shared/studiomodel/studio.h"

namespace hlmv
{
class CHLMV;

class I3DViewListener
{
public:
	virtual ~I3DViewListener() = 0;

	virtual void Draw3D( const wxSize& size ) = 0;
};

inline I3DViewListener::~I3DViewListener()
{
}

class C3DView final : public ui::CwxBaseGLCanvas
{
public:
	C3DView( wxWindow* pParent, CHLMV* const pHLMV, I3DViewListener* pListener = nullptr );
	~C3DView();

	const CHLMV* GetHLMV() const { return m_pHLMV; }
	CHLMV* GetHLMV() { return m_pHLMV; }

	void MouseEvents( wxMouseEvent& event );

	//Tells the 3D view to prepare for model loading.
	void PrepareForLoad();

	void UpdateView();

	bool LoadBackgroundTexture( const wxString& szFilename );
	void UnloadBackgroundTexture();

	bool LoadGroundTexture( const wxString& szFilename );
	void UnloadGroundTexture();

	void SaveUVMap( const wxString& szFilename, const int iTexture );

	void TakeScreenshot();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void DrawScene() override final;

	void SetupRenderMode( RenderMode renderMode = RenderMode::INVALID );

	void DrawTexture( const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines, const mstudiomesh_t* const pUVMesh );

	void DrawModel();

private:
	CHLMV* const m_pHLMV;

	I3DViewListener* m_pListener;

	GLuint m_BackgroundTexture	= GL_INVALID_TEXTURE_ID;
	GLuint m_GroundTexture		= GL_INVALID_TEXTURE_ID;

	float m_flOldRotX = 0;
	float m_flOldRotY = 0;

	glm::vec3 m_vecOldTrans = { 0, 0, 0 };

	float m_flOldX = 0;
	float m_flOldY = 0;

	//Tracks mouse button state. Used to prevent input from being mistakingly applied (e.g. prevent double click from dialog spilling over as drag).
	int m_iButtonsDown = wxMOUSE_BTN_NONE;

private:
	C3DView( const C3DView& ) = delete;
	C3DView& operator=( const C3DView& ) = delete;
};
}

#endif //C3DVIEW_H