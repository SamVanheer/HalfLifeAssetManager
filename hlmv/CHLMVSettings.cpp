#include "model/studiomodel/StudioModel.h"

#include "CHLMVSettings.h"

CHLMVSettings Options;

CHLMVSettings::CHLMVSettings()
	: m_pStudioModel( nullptr )
{
	ResetToDefaults();
}

CHLMVSettings::~CHLMVSettings()
{
	ClearStudioModel();
}

void CHLMVSettings::ResetModelData()
{
	trans[ 0 ] = trans[ 1 ] = trans[ 2 ] = 0;
	rot[ 1 ] = rot[ 2 ] = 0;

	rot[ 0 ] = -90.0f;

	weaponOrigin[ 0 ] = weaponOrigin[ 1 ] = weaponOrigin[ 2 ] = 0;

	texture = 0;

	pUVMesh = nullptr;
}

void CHLMVSettings::ResetToDefaults()
{
	ResetModelData();

	showBones = false;

	lightColor[ 0 ] = lightColor[ 1 ] = lightColor[ 2 ] = 1.0;

	transparency = 1.0f;

	showAttachments = false;
	showHitBoxes = false;
	showTexture = false;

	textureScale = 1.0f;

	memset( backgroundTextureFile, 0, sizeof( backgroundTextureFile ) );
	memset( groundTextureFile, 0, sizeof( groundTextureFile ) );

	//There used to be an option called "useStencil" that was enabled along with mirror if the OpenGL driver was not 3DFX.
	//Considering how old that driver (and the hardware that used it) is, the code for it was removed.
	//The stencil buffer is now always used by mirror (it limits the mirroring effect to the floor quad).
	mirror = false;

	renderMode = RenderMode::TEXTURE_SHADED;

	showGround = false;

	groundColor[ 0 ] = 0.85f;
	groundColor[ 1 ] = 0.85f;
	groundColor[ 2 ] = 0.69f;

	pause = false;

	speedScale = 1.0f;

	playSequence = true;

	sequence = 0;

	drawnPolys = 0;

	showEyePosition = false;

	wireframeOverlay = false;

	wireframeColor[ 0 ] = 1.0f;
	wireframeColor[ 1 ] = wireframeColor[ 2 ] = 0;

	useWeaponOrigin = false;

	showUVMap = false;

	overlayUVMap = false;

	antiAliasUVLines = false;
}

void CHLMVSettings::CenterView( const StudioModel& model )
{
	float min[ 3 ], max[ 3 ];
	model.ExtractBbox( min, max );

	float dx = max[ 0 ] - min[ 0 ];
	float dy = max[ 1 ] - min[ 1 ];
	float dz = max[ 2 ] - min[ 2 ];

	float d = dx;

	if( dy > d )
		d = dy;
	if( dz > d )
		d = dz;

	trans[ 0 ] = 0;
	trans[ 1 ] = min[ 2 ] + dz / 2;
	trans[ 2 ] = d * 1.0f;
	rot[ 0 ] = -90.0f;
	rot[ 1 ] = -90.0f;
	rot[ 2 ] = 0.0f;
}

void CHLMVSettings::SetOrigin( const vec3_t vecOrigin )
{
	VectorCopy( vecOrigin, trans );
}

void CHLMVSettings::ClearStudioModel()
{
	SetStudioModel( nullptr );
}

void CHLMVSettings::SetStudioModel( StudioModel* pStudioModel )
{
	if( m_pStudioModel )
	{
		delete m_pStudioModel;
		m_pStudioModel = nullptr;
	}

	if( pStudioModel )
		m_pStudioModel = pStudioModel;
}