#include "lib/LibInterface.h"

#include "cvar/CVar.h"

#include "CRendererLibrary.h"

namespace renderer
{
REGISTER_SINGLE_INTERFACE( IRENDERERLIBRARY_NAME, CRendererLibrary );

bool CRendererLibrary::Connect( const CreateInterfaceFn* const pFactories, const size_t uiNumFactories )
{
	for( size_t uiIndex = 0; uiIndex < uiNumFactories; ++uiIndex )
	{
		if( !g_pCVar )
		{
			g_pCVar = static_cast<cvar::ICVarSystem*>( pFactories[ uiIndex ]( ICVARSYSTEM_NAME, nullptr ) );
		}
	}

	if( !g_pCVar )
		return false;

	cvar::ConnectCVars();

	return true;
}

void CRendererLibrary::Disconnect()
{
}
}