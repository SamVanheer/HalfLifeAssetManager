#include "CHLMVState.h"

namespace hlmv
{
const Vector CHLMVState::DEFAULT_ROTATION = Vector( -90.0f, 0, 0 );

CHLMVState::CHLMVState()
	: m_pStudioModel( nullptr )
{
	ResetToDefaults();
}

CHLMVState::~CHLMVState()
{
	ClearStudioModel();
}

void CHLMVState::ResetModelData()
{
	trans[ 0 ] = trans[ 1 ] = trans[ 2 ] = 0;

	rot = DEFAULT_ROTATION;

	weaponOrigin[ 0 ] = weaponOrigin[ 1 ] = weaponOrigin[ 2 ] = 0;

	texture = 0;

	pUVMesh = nullptr;
}

void CHLMVState::ResetToDefaults()
{
	ResetModelData();

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

	pause = false;

	playSequence = true;

	sequence = 0;

	playSound = false;

	drawnPolys = 0;

	wireframeOverlay = false;

	useWeaponOrigin = false;

	showUVMap = false;

	overlayUVMap = false;

	antiAliasUVLines = false;

	renderSettings.ResetToDefaults();
}

void CHLMVState::CenterView()
{
	if( !m_pStudioModel )
		return;

	float min[ 3 ], max[ 3 ];
	m_pStudioModel->ExtractBbox( min, max );

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

void CHLMVState::SaveView()
{
	savedTrans	= trans;
	savedRot	= rot;
}

void CHLMVState::RestoreView()
{
	trans	= savedTrans;
	rot		= savedRot;
}

void CHLMVState::SetOrigin( const Vector& vecOrigin )
{
	trans = vecOrigin;
}

void CHLMVState::ClearStudioModel()
{
	SetStudioModel( nullptr );
}

void CHLMVState::SetStudioModel( StudioModel* pStudioModel )
{
	if( m_pStudioModel )
	{
		delete m_pStudioModel;
		m_pStudioModel = nullptr;
	}

	if( pStudioModel )
		m_pStudioModel = pStudioModel;
}
}