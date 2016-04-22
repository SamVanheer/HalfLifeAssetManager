#ifndef HLMV_CHLMVSTATE_H
#define HLMV_CHLMVSTATE_H

#include <vector>
#include <memory>

#include <glm/vec3.hpp>

#include "common/Platform.h"
#include "graphics/OpenGL.h"
#include "graphics/Constants.h"

#include "game/studiomodel/StudioModel.h"

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
	static const glm::vec3 DEFAULT_ROTATION;

public:
	CHLMVState();
	~CHLMVState();

	void ResetModelData();

	void ResetToDefaults();

	void CenterView();

	void SaveView();

	void RestoreView();

	void SetOrigin( const glm::vec3& vecOrigin );

	StudioModel* GetStudioModel() { return m_pStudioModel; }
	const StudioModel* GetStudioModel() const { return m_pStudioModel; }

	void ClearStudioModel();

	void SetStudioModel( StudioModel* pStudioModel );

	bool DumpModelInfo( const char* const pszFilename );

public:
	glm::vec3 trans;
	glm::vec3 rot;

	//Do not reset these; the user inits these!
	glm::vec3 savedTrans;
	glm::vec3 savedRot = DEFAULT_ROTATION;

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

	glm::vec3 weaponOrigin;

	bool useWeaponOrigin;

	bool showUVMap;

	bool overlayUVMap;

	const mstudiomesh_t* pUVMesh;	//Null if all should be drawn.

	bool antiAliasUVLines;

private:
	StudioModel* m_pStudioModel;

private:
	CHLMVState( const CHLMVState& ) = delete;
	CHLMVState& operator=( const CHLMVState& other ) = delete;
};
}

#endif //HLMV_CHLMVSTATE_H