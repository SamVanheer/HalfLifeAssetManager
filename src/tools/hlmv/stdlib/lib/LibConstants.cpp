#include <cassert>

#include "shared/Utility.h"

#include "LibConstants.h"

namespace
{
static const CPlatform g_UnknownPlatform( Platform::UNKNOWN, "Unknown", "", "", nullptr, 0 );

static const char* const g_WindowsLibExts[] =
{
	".dll"
};

static const CPlatform g_WindowsPlatform( Platform::WINDOWS, "Windows", "", g_WindowsLibExts[ 0 ], g_WindowsLibExts, ARRAYSIZE( g_WindowsLibExts ) );

//Can add common extensions here if needed to avoid complicating things. - Solokiller
static const char* const g_LinuxLibExts[] = 
{
	".so",
};

static const CPlatform g_LinuxPlatform( Platform::LINUX, "Linux", "lib", g_LinuxLibExts[ 0 ], g_LinuxLibExts, ARRAYSIZE( g_LinuxLibExts ) );

static const char* const g_MacLibExts[] =
{
	".dylib"
};

static const CPlatform g_MacPlatform( Platform::MAC, "Mac", "lib", g_MacLibExts[ 0 ], g_MacLibExts, ARRAYSIZE( g_MacLibExts ) );
}

const CPlatform& CPlatform::GetPlatform( const Platform platform )
{
	switch( platform )
	{
	default:
		assert( false );
	case Platform::UNKNOWN:		return g_UnknownPlatform;
	case Platform::WINDOWS:		return g_WindowsPlatform;
	case Platform::LINUX:		return g_LinuxPlatform;
	case Platform::MAC:			return g_MacPlatform;
	}
}

const CPlatform& CPlatform::GetCurrentPlatform()
{
	return GetPlatform( ::GetCurrentPlatform() );
}

CPlatform::CPlatform( 
	const Platform platform,
	const char* const pszName, 
	const char* const pszDefaultLibPrefix, const char* const pszDefaultLibExt,
	const char* const* const pszLibExts, const size_t uiNumLibExts )
	: m_Platform( platform )
	, m_pszName( pszName )
	, m_pszDefaultLibPrefix( pszDefaultLibPrefix )
	, m_pszDefaultLibExt( pszDefaultLibExt )
	, m_pszLibExts( pszLibExts )
	, m_uiNumLibExts( uiNumLibExts )
{
	assert( pszName );
	assert( pszDefaultLibPrefix );
	assert( pszDefaultLibExt );
	assert( uiNumLibExts == 0 || pszLibExts );
}

const char* CPlatform::GetLibExt( const size_t uiIndex ) const
{
	assert( uiIndex < m_uiNumLibExts );

	return m_pszLibExts[ uiIndex ];
}