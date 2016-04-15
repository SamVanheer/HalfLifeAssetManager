#ifndef HLMV_CHLMVSTATE_H
#define HLMV_CHLMVSTATE_H

#include <vector>
#include <memory>

#include "common/Platform.h"
#include "graphics/OpenGL.h"
#include "graphics/Constants.h"

#include "utility/Vector.h"

#include "studiomodel/StudioModel.h"

#include "settings/CGameConfig.h"
#include "settings/CGameConfigManager.h"
#include "settings/CRecentFiles.h"

/*
*	Contains all settings used by the HLMV application.
*/
namespace hlmv
{
class CHLMVState final
{
public:
	CHLMVState();
	~CHLMVState();

	void ResetModelData();

	void ResetToDefaults();

	void CenterView( const StudioModel& model );

	void SetOrigin( const Vector& vecOrigin );

	StudioModel* GetStudioModel() { return m_pStudioModel; }
	const StudioModel* GetStudioModel() const { return m_pStudioModel; }

	void ClearStudioModel();

	void SetStudioModel( StudioModel* pStudioModel );

public:
	Vector trans;
	Vector rot;

	bool showBackground;

	bool showTexture;

	//Index of the texture to draw onscreen in Texture mode.
	int texture;

	float textureScale;

	char backgroundTextureFile[ MAX_PATH_LENGTH ];
	char groundTextureFile[ MAX_PATH_LENGTH ];

	bool mirror;

	RenderMode renderMode;

	bool showGround;

	bool pause;

	bool playSequence;

	int sequence;

	bool playSound;

	/**
	*	How many polygons were drawn in the last frame. Does not include things like hitboxes or attachments, only the actual model.
	*/
	unsigned int drawnPolys;

	bool wireframeOverlay;

	Vector weaponOrigin;

	bool useWeaponOrigin;

	bool showUVMap;

	bool overlayUVMap;

	const mstudiomesh_t* pUVMesh;	//Null if all should be drawn.

	bool antiAliasUVLines;

	StudioModel::CRenderSettings renderSettings;

private:
	StudioModel* m_pStudioModel;

private:
	CHLMVState( const CHLMVState& ) = delete;
	CHLMVState& operator=( const CHLMVState& other ) = delete;
};
}

#endif //HLMV_CHLMVSTATE_H