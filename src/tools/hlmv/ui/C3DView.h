#ifndef C3DVIEW_H
#define C3DVIEW_H

#include "wxHLMV.h"

#include "common/CwxBase3DView.h"

#include <glm/vec3.hpp>

#include "graphics/Constants.h"
#include "graphics/CCamera.h"

#include "shared/studiomodel/studio.h"

class CStudioModelEntity;

namespace hlmv
{
class CModelViewerApp;
class CMainPanel;

class C3DView final : public ui::CwxBase3DView
{
private:
	static const int CROSSHAIR_LINE_WIDTH = 3;
	static const int CROSSHAIR_LINE_START = 5;
	static const int CROSSHAIR_LINE_LENGTH = 10;
	static const int CROSSHAIR_LINE_END = CROSSHAIR_LINE_START + CROSSHAIR_LINE_LENGTH;

	static const int GUIDELINES_LINE_WIDTH = 1;
	static const int GUIDELINES_LINE_LENGTH = 5;
	static const int GUIDELINES_POINT_LINE_OFFSET = 2;
	static const int GUIDELINES_OFFSET = GUIDELINES_LINE_LENGTH + (GUIDELINES_POINT_LINE_OFFSET * 2) + GUIDELINES_LINE_WIDTH;

	static const int GUIDELINES_EDGE_WIDTH = 4;

public:
	C3DView( wxWindow* pParent, CModelViewerApp* const pHLMV, CMainPanel* const pMainPanel);
	~C3DView();

	const CModelViewerApp* GetHLMV() const { return m_pHLMV; }
	CModelViewerApp* GetHLMV() { return m_pHLMV; }

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

	/**
	*	Applies the current camera settings to the scene.
	*/
	void ApplyCameraToScene();

	void MouseEvents( wxMouseEvent& event );

	void SetupRenderMode( RenderMode renderMode = RenderMode::INVALID );

	void DrawTexture( const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines, const mstudiomesh_t* const pUVMesh );

	/**
	*	Draws a texture onto the screen. Optionally draws a UV map, either on a black background, or on top of the texture.
	*	@param iXOffset			X Offset.
	*	@param iYOffset			Y Offset.
	*	@param iWidth			Width of the viewport
	*	@param iHeight			Height of the viewport
	*	@param pEntity			Entity whose model's texture is being drawn
	*	@param iTexture			Index of the texture to draw
	*	@param flTextureScale	Zoom level
	*	@param bShowUVMap		If true, draws the UV map
	*	@param bOverlayUVMap	If true, and bShowUVMap is true, overlays the UV map on top of the texture
	*	@param bAntiAliasLines	If true, anti aliases UV map lines
	*	@param pUVMesh			If specified, is the mesh to use to draw the UV map. If null, all meshes that use the texture are drawn.
	*/
	void DrawTexture( const int iXOffset, const int iYOffset, const int iWidth, const int iHeight,
					  CStudioModelEntity* pEntity,
					  const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines,
					  const mstudiomesh_t* const pUVMesh );


	void DrawModel();

private:
	CModelViewerApp* const m_pHLMV;

	CMainPanel* const m_pMainPanel;

	//Used for rotation and translation.
	graphics::CCamera m_OldCamera;

	//Old mouse coordinates.
	glm::vec2 m_vecOldCoords;

	//Tracks mouse button state. Used to prevent input from being mistakingly applied (e.g. prevent double click from dialog spilling over as drag).
	int m_iButtonsDown = wxMOUSE_BTN_NONE;

	//Whether the mouse data is for the textures panel or not.
	bool m_bTexPanelMouseData = false;

	float m_flOldTextureScale;

	GLuint m_BackgroundTexture	= GL_INVALID_TEXTURE_ID;
	GLuint m_GroundTexture		= GL_INVALID_TEXTURE_ID;

private:
	C3DView( const C3DView& ) = delete;
	C3DView& operator=( const C3DView& ) = delete;
};
}

#endif //C3DVIEW_H