#include "common/Logging.h"

#include "keyvalues/CKeyvaluesParser.h"
#include "keyvalues/CKeyvaluesWriter.h"
#include "keyvalues/CKeyvalues.h"
#include "keyvalues/CKeyvalueNode.h"
#include "keyvalues/CKeyvalue.h"
#include "keyvalues/CKvBlockNode.h"

#include "settings/GameConfigIO.h"

#include "CHLMVSettings.h"

CHLMVSettings::CHLMVSettings()
	: m_pStudioModel( nullptr )
{
	ResetToDefaults();
}

CHLMVSettings::~CHLMVSettings()
{
	//Note: don't clear the studio model here; since the same instance is shared.
	//TODO: use reference counted/shared ptr to avoid this problem
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

	playSound = false;

	drawnPolys = 0;

	wireframeOverlay = false;

	useWeaponOrigin = false;

	showUVMap = false;

	overlayUVMap = false;

	antiAliasUVLines = false;

	renderSettings.ResetToDefaults();

	configManager->RemoveAllConfigs();
}

bool CHLMVSettings::LoadFromFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	CKeyvaluesParser parser( pszFilename );

	if( !parser.HasInputData() )
		return false;

	const CKeyvaluesParser::ParseResult result = parser.Parse();

	if( result == CKeyvaluesParser::Success )
	{
		auto keyvalues = parser.GetKeyvalues();

		auto root = keyvalues->GetRoot();

		auto configs = root->FindFirstChild<CKvBlockNode>( "gameConfigs" );

		if( configs )
		{
			const auto result = settings::LoadGameConfigs( *configs, configManager );

			if( result.first < result.second )
				Warning( "%u game configurations failed to load\n", result.second - result.first );

			auto settings = root->FindFirstChild<CKvBlockNode>( "hlmvSettings" );

			if( settings )
			{
				auto active = settings->FindFirstChild<CKeyvalue>( "activeConfig" );

				if( active )
				{
					configManager->SetActiveConfig( active->GetValue().CStr() );
				}
			}
		}

		return true;
	}

	return false;
}

bool CHLMVSettings::SaveToFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	CKeyvaluesWriter writer( pszFilename );

	if( writer.IsOpen() )
	{
		writer.BeginBlock( "hlmvSettings" );

		if( auto activeConfig = configManager->GetActiveConfig() )
		{
			writer.WriteKeyvalue( "activeConfig", activeConfig->GetName() );
		}

		writer.EndBlock();

		std::shared_ptr<CKvBlockNode> gameConfigs = std::make_shared<CKvBlockNode>( "gameConfigs" );

		settings::SaveGameConfigs( configManager, *gameConfigs );

		writer.WriteBlock( *gameConfigs );

		writer.Close();

		return true;
	}

	return false;
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