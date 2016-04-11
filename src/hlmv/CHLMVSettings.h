#ifndef HLMV_CHLMVSETTINGS_H
#define HLMV_CHLMVSETTINGS_H

#include "common/Platform.h"
#include "graphics/OpenGL.h"
#include "graphics/Constants.h"

#include "utility/mathlib.h"

#include "studiomodel/StudioModel.h"

/*
*	Contains all settings used by the HLMV application.
*/
class CHLMVSettings final
{
public:
	CHLMVSettings();
	~CHLMVSettings();

	CHLMVSettings( const CHLMVSettings& other ) = default;
	CHLMVSettings& operator=( const CHLMVSettings& other ) = default;

	void ResetModelData();

	void ResetToDefaults();

	void CenterView( const StudioModel& model );

	void SetOrigin( const vec3_t vecOrigin );

	StudioModel* GetStudioModel() { return m_pStudioModel; }
	const StudioModel* GetStudioModel() const { return m_pStudioModel; }

	void ClearStudioModel();

	void SetStudioModel( StudioModel* pStudioModel );

public:
	//TODO: replace with Vector
	vec3_t trans;
	vec3_t rot;

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

	vec3_t groundColor;

	bool pause;

	float speedScale;

	bool playSequence;

	int sequence;

	bool playSound;

	unsigned int drawnPolys; //TODO: move

	bool wireframeOverlay;

	vec3_t weaponOrigin;

	bool useWeaponOrigin;

	bool showUVMap;

	bool overlayUVMap;

	const mstudiomesh_t* pUVMesh;	//Null if all should be drawn.

	bool antiAliasUVLines;

	StudioModel::CRenderSettings renderSettings;

private:
	StudioModel* m_pStudioModel;
};

#endif //HLMV_CHLMVSETTINGS_H