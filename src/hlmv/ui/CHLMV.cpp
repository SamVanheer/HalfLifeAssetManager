#include "CMainWindow.h"

#include "CHLMV.h"

namespace hlmv
{
CHLMV::CHLMV( CHLMVSettings* const pSettings, CHLMVState* const pState )
	: m_pSettings( pSettings )
	, m_pState( pState )
{
}

CHLMV::~CHLMV()
{
}

void CHLMV::SetMainWindow( CMainWindow* const pMainWindow )
{
	m_pMainWindow = pMainWindow;
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