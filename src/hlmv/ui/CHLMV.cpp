#include "CMainWindow.h"

#include "CHLMV.h"

namespace hlmv
{
CHLMV::CHLMV()
	: m_pSettings( new CHLMVSettings() )
	, m_pState( new CHLMVState() )
{
}

CHLMV::~CHLMV()
{
	delete m_pState;
	delete m_pSettings;
}

void CHLMV::SetMainWindow( CMainWindow* const pMainWindow )
{
	m_pMainWindow = pMainWindow;
}

bool CHLMV::Initialize()
{
	if( !m_pSettings->Initialize( HLMV_SETTINGS_FILE ) )
	{
		return false;
	}

	return true;
}

void CHLMV::Shutdown()
{
	m_pSettings->Shutdown( HLMV_SETTINGS_FILE );
}

bool CHLMV::LoadModel( const char* const pszFilename )
{
	return m_pMainWindow->LoadModel( pszFilename );
}

bool CHLMV::PromptLoadModel()
{
	return m_pMainWindow->PromptLoadModel();
}

bool CHLMV::SaveModel( const char* const pszFilename )
{
	return m_pMainWindow->SaveModel( pszFilename );
}

bool CHLMV::PromptSaveModel()
{
	return m_pMainWindow->PromptSaveModel();
}

bool CHLMV::LoadBackgroundTexture( const char* const pszFilename )
{
	return m_pMainWindow->LoadBackgroundTexture( pszFilename );
}

bool CHLMV::PromptLoadBackgroundTexture()
{
	return m_pMainWindow->PromptLoadBackgroundTexture();
}

void CHLMV::UnloadBackgroundTexture()
{
	m_pMainWindow->UnloadBackgroundTexture();
}

bool CHLMV::LoadGroundTexture( const char* const pszFilename )
{
	return m_pMainWindow->LoadGroundTexture( pszFilename );
}

bool CHLMV::PromptGroundTexture()
{
	return m_pMainWindow->PromptLoadGroundTexture();
}

void CHLMV::UnloadGroundTexture()
{
	m_pMainWindow->UnloadGroundTexture();
}

void CHLMV::SaveUVMap( const wxString& szFilename, const int iTexture )
{
	m_pMainWindow->SaveUVMap( szFilename, iTexture );
}
}