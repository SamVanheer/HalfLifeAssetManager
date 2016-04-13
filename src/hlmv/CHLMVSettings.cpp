#include "common/Logging.h"

#include "keyvalues/CKeyvaluesParser.h"
#include "keyvalues/CKeyvaluesWriter.h"
#include "keyvalues/CKeyvalues.h"
#include "keyvalues/CKeyvalueNode.h"
#include "keyvalues/CKeyvalue.h"
#include "keyvalues/CKvBlockNode.h"

#include "filesystem/CFileSystem.h"

#include "settings/GameConfigIO.h"

#include "CHLMVSettings.h"

#define HLMV_SETTINGS_FILE "HLMVSettings.txt"

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

				if( auto block = settings->FindFirstChild<CKvBlockNode>( "recentFiles" ) )
				{
					const auto& children = block->GetChildren();

					for( const auto& child : children )
					{
						if( child->GetType() != KVNode_Keyvalue )
							continue;

						if( child->GetKey() != "recentFile" )
							continue;

						auto file = std::static_pointer_cast<CKeyvalue>( child );

						recentFiles->Add( file->GetValue().CStr() );
					}
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

		writer.BeginBlock( "recentFiles" );

		for( const auto& file : recentFiles->GetFiles() )
		{
			writer.WriteKeyvalue( "recentFile", file.c_str() );
		}

		writer.EndBlock();

		writer.EndBlock();

		std::shared_ptr<CKvBlockNode> gameConfigs = std::make_shared<CKvBlockNode>( "gameConfigs" );

		settings::SaveGameConfigs( configManager, *gameConfigs );

		writer.WriteBlock( *gameConfigs );

		writer.Close();

		return true;
	}

	return false;
}

bool CHLMVSettings::Initialize()
{
	if( m_bInitialized )
		return false;

	m_bInitialized = true;

	bool bResult = LoadFromFile( HLMV_SETTINGS_FILE );

	//Load the settings, or if the file didn't exist, save settings.
	if( !bResult )
	{
		bResult = SaveToFile( HLMV_SETTINGS_FILE );
	}

	if( bResult )
	{
		bResult = InitializeFileSystem();
	}

	return bResult;
}

void CHLMVSettings::Shutdown()
{
	if( !m_bInitialized )
		return;

	m_bInitialized = false;

	if( !SaveToFile( HLMV_SETTINGS_FILE ) )
		Error( "Failed to save settings!\n" );
}

bool CHLMVSettings::InitializeFileSystem()
{
	if( auto activeConfig = configManager->GetActiveConfig() )
	{
		fileSystem().SetBasePath( activeConfig->GetBasePath() );

		fileSystem().RemoveAllSearchPaths();

		CString szPath;

		const char* pszDirs[] =
		{
			"",
			"_downloads",
			"_addon",
			"_hd"
		};

		//Note: do not use a reference here. Varargs doesn't convert it, so it'll end up being const char**.

		//Add mod dirs first, since they override game dirs.
		if( strcmp( activeConfig->GetGameDir(), activeConfig->GetModDir() ) )
		{
			for( const auto pszDir : pszDirs )
			{
				szPath.Format( "%s%s", activeConfig->GetModDir(), pszDir );

				fileSystem().AddSearchPath( szPath.CStr() );
			}
		}

		for( const auto pszDir : pszDirs )
		{
			szPath.Format( "%s%s", activeConfig->GetGameDir(), pszDir );

			fileSystem().AddSearchPath( szPath.CStr() );
		}
	}

	return true;
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