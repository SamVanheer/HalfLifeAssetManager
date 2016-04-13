#ifndef HLMV_CHLMVSETTINGS_H
#define HLMV_CHLMVSETTINGS_H

#include <vector>
#include <memory>

#include "common/Platform.h"
#include "graphics/OpenGL.h"
#include "graphics/Constants.h"

#include "utility/mathlib.h"

#include "studiomodel/StudioModel.h"

#include "settings/CGameConfig.h"
#include "settings/CGameConfigManager.h"
#include "settings/CRecentFiles.h"

/*
*	Contains all settings used by the HLMV application.
*/
class CHLMVSettings final
{
public:
	static const size_t MAX_RECENT_FILES = 4;

public:
	//TODO: split into viewer specific and general purpose
	CHLMVSettings();
	~CHLMVSettings();

	void ResetModelData();

	void ResetToDefaults();

	bool LoadFromFile( const char* const pszFilename );
	bool SaveToFile( const char* const pszFilename );

	bool Initialize();
	void Shutdown();

	bool InitializeFileSystem();

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

	std::shared_ptr<settings::CGameConfigManager> configManager = std::make_shared<settings::CGameConfigManager>();

	std::shared_ptr<settings::CRecentFiles> recentFiles = std::make_shared<settings::CRecentFiles>( MAX_RECENT_FILES );

private:
	StudioModel* m_pStudioModel;

	bool m_bInitialized = false;

private:
	CHLMVSettings( const CHLMVSettings& other ) = default;
	CHLMVSettings& operator=( const CHLMVSettings& other ) = delete;
};

#endif //HLMV_CHLMVSETTINGS_H