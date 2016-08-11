#include "ui/wx/wxInclude.h"

#include "core/shared/Platform.h"

#include "CBaseWXToolApp.h"

namespace tools
{
bool CBaseWXToolApp::RunApp( int iArgc, wchar_t* pszArgV[] )
{
	if( !PreRunApp( iArgc, pszArgV ) )
		return false;

	return RunWXApp( iArgc, pszArgV );
}

bool CBaseWXToolApp::RunWXApp( int iArgc, wchar_t* pszArgV[] )
{
	int iReturnCode;

#ifdef __WXMSW__
	//TODO: could probably pass this in instead of using a hack. - Solokiller
	iReturnCode = wxEntry( static_cast<HINSTANCE>( HINST_THISCOMPONENT ), NULL, nullptr, 1 );
#else
	iReturnCode = wxEntry( iArgc, pszArgV );
#endif

	return iReturnCode == 0;
}
}