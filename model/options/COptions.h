#ifndef OPTIONS_COPTIONS_H
#define OPTIONS_COPTIONS_H

#include "model/utility/Platform.h"
#include "model/utility/OpenGL.h"

#include "mathlib.h"

//TODO: move
enum class RenderMode
{
	INVALID				= -1,
	FIRST				= 0,

	WIREFRAME			= FIRST,
	FLAT_SHADED,
	SMOOTH_SHADED,
	TEXTURE_SHADED,

	COUNT,
	LAST				= COUNT - 1 //Must be last
};

class StudioModel;

/*
*	Contains all options used by the application.
*/
class COptions final
{
public:
	COptions();
	~COptions();

	void ResetToDefaults();

	void CenterView( const StudioModel& model );

	void SetOrigin( const vec3_t vecOrigin );

public:
	bool showBones;

	//TODO: replace with Vector
	vec3_t lightColor;
	vec3_t trans;
	vec3_t rot;

	float transparency;

	bool showAttachments;

	bool showHitBoxes;

	bool useStencil;

	bool showBackground;

	bool showTexture;

	//Index of the texture to draw onscreen in Texture mode.
	GLuint texture;

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

private:
	COptions( const COptions& ) = delete;
	COptions& operator=( const COptions& ) = delete;
};

/*
*	Global instance.
*/
extern COptions Options;

#endif //OPTIONS_COPTIONS_H