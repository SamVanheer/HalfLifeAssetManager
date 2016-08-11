#include "ui/wx/wxInclude.h"

#include "core/shared/Platform.h"

#include "ui/wx/CwxOpenGL.h"

#include "CBaseWXToolApp.h"

namespace tools
{
bool CBaseWXToolApp::Connect( const CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	if( !CBaseToolApp::Connect( pFactories, uiNumFactories ) )
		return false;

	wxInitAllImageHandlers();

	return true;
}

bool CBaseWXToolApp::RunApp( int iArgc, wchar_t* pszArgV[] )
{
	if( !PreRunApp( iArgc, pszArgV ) )
		return false;

	return RunWXApp( iArgc, pszArgV );
}

bool CBaseWXToolApp::InitOpenGL()
{
	//Set up OpenGL parameters.
	CwxOpenGL::CreateInstance();

	wxGLAttributes canvasAttributes;

	GetGLCanvasAttributes( canvasAttributes );

	wxGLContextAttrs contextAttributes;

	GetGLContextAttributes( contextAttributes );

	if( !wxOpenGL().Initialize( canvasAttributes, &contextAttributes ) )
	{
		wxMessageBox( "Failed to initialize OpenGL", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	return true;
}

void CBaseWXToolApp::GetGLCanvasAttributes( wxGLAttributes& attrs )
{
	attrs
		.PlatformDefaults()
		.RGBA()
		.Depth( 16 )
		.Stencil( 8 )
		.DoubleBuffer()
		.EndList();
}

void CBaseWXToolApp::GetGLContextAttributes( wxGLContextAttrs& attrs )
{
	//The default settings for OpenGL allow for Windows XP support. OpenGL 2.1 is typically supported by XP era hardware, making it the best choice.
	//2.1 supports GLSL 120, which has some features that are nice to have in shaders.
	attrs
		.PlatformDefaults()
		.MajorVersion( 2 )
		.MinorVersion( 1 )
		.EndList();
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