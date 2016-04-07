#ifndef HLMV_CHLMVSETTINGS_H
#define HLMV_CHLMVSETTINGS_H

#include "model/utility/Platform.h"
#include "model/utility/OpenGL.h"
#include "model/graphics/Constants.h"

#include "mathlib.h"

#include "model/studiomodel/StudioModel.h"

class StudioModel;

/*
*	Contains all settings used by the HLMV application.
*/
class CHLMVSettings final
{
public:
	CHLMVSettings();
	~CHLMVSettings();

	void ResetModelData();

	void ResetToDefaults();

	void CenterView( const StudioModel& model );

	void SetOrigin( const vec3_t vecOrigin );

	StudioModel* GetStudioModel() { return m_pStudioModel; }
	const StudioModel* GetStudioModel() const { return m_pStudioModel; }

	void ClearStudioModel();

	void SetStudioModel( StudioModel* pStudioModel );

public:
	bool showBones;

	//TODO: replace with Vector
	vec3_t lightColor;
	vec3_t trans;
	vec3_t rot;

	float transparency;

	bool showAttachments;

	bool showHitBoxes;

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

	unsigned int drawnPolys; //TODO: move

	bool showEyePosition;

	bool wireframeOverlay;

	vec3_t wireframeColor;

	vec3_t weaponOrigin;

	bool useWeaponOrigin;

	bool showUVMap;

	bool overlayUVMap;

	const mstudiomesh_t* pUVMesh;	//Null if all should be drawn.

	bool antiAliasUVLines;

private:
	StudioModel* m_pStudioModel;

private:
	CHLMVSettings( const CHLMVSettings& ) = delete;
	CHLMVSettings& operator=( const CHLMVSettings& ) = delete;
};

/*
*	Global instance.
*/
extern CHLMVSettings Options;

#endif //HLMV_CHLMVSETTINGS_H