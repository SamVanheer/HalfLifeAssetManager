#ifndef C3DVIEW_H
#define C3DVIEW_H

#include "wxHLMV.h"

#include "ui/wx/shared/CwxBase3DView.h"

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

class C3DView final : public ui::CwxBase3DView
{
public:
	C3DView( wxWindow* pParent, CHLMV* const pHLMV, I3DViewListener* pListener = nullptr );
	~C3DView();

	const CHLMV* GetHLMV() const { return m_pHLMV; }
	CHLMV* GetHLMV() { return m_pHLMV; }

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
	void OnDraw() override final;

	void MouseEvents( wxMouseEvent& event ) override final;

	bool LeftMouseDrag( wxMouseEvent& event ) override final;

	void SetupRenderMode( RenderMode renderMode = RenderMode::INVALID );

	void DrawTexture( const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines, const mstudiomesh_t* const pUVMesh );

	void DrawModel();

private:
	CHLMV* const m_pHLMV;

	I3DViewListener* m_pListener;

	GLuint m_BackgroundTexture	= GL_INVALID_TEXTURE_ID;
	GLuint m_GroundTexture		= GL_INVALID_TEXTURE_ID;

private:
	C3DView( const C3DView& ) = delete;
	C3DView& operator=( const C3DView& ) = delete;
};
}

#endif //C3DVIEW_H