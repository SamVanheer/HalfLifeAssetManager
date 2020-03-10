#ifndef LIB_CLIBARGS_H
#define LIB_CLIBARGS_H

#include <cassert>

#include "LibConstants.h"

/**
*	Builder for CLibrary load calls. Lets you specify platform specific settings as needed.
*/
class CLibArgs final
{
public:
	/**
	*	Constructor.
	*	@param pszFilename The base library filename. This is the name without the "lib" prefix (for Unix systems), and without any extension.
	*/
	CLibArgs( const char* const pszFilename )
		: m_pszFilename( pszFilename )
	{
		assert( pszFilename );
	}

	CLibArgs( CLibArgs&& ) = default;
	CLibArgs& operator=( CLibArgs&& ) = default;

	/**
	*	@return The filename.
	*/
	const char* GetFilename() const { return m_pszFilename; }

	/**
	*	Sets the library filename for the given platform.
	*/
	CLibArgs& Filename( const char* const pszFilename, const Platform platform )
	{
		if( GetCurrentPlatform() == platform )
			m_pszFilename = pszFilename;

		return *this;
	}

	/**
	*	@return The path.
	*/
	const char* GetPath() const { return m_pszPath; }

	/**
	*	Sets the path to look in for the library for the given platform.
	*/
	CLibArgs& Path( const char* const pszPath, const Platform platform )
	{
		if( GetCurrentPlatform() == platform )
			m_pszPath = pszPath;

		return *this;
	}

	/**
	*	Sets the path to look in for the library.
	*/
	CLibArgs& Path( const char* const pszPath )
	{
		m_pszPath = pszPath;

		return *this;
	}

	/**
	*	@return Whether to disable platform specific prefixes.
	*/
	bool ShouldDisablePrefixes() const { return m_bDisablePrefix; }

	/**
	*	Sets whether prefixes are disabled for the given platform.
	*	@param bDisablePrefixes Whether to disable prefixes.
	*	@param platform Platform.
	*/
	CLibArgs& DisablePrefixes( const bool bDisablePrefixes, const Platform platform )
	{
		if( GetCurrentPlatform() == platform )
			m_bDisablePrefix = bDisablePrefixes;

		return *this;
	}

	/**
	*	Sets whether prefixes are disabled.
	*	@param bDisablePrefixes Whether to disable prefixes.
	*/
	CLibArgs& DisablePrefixes( const bool bDisablePrefixes )
	{
		m_bDisablePrefix = bDisablePrefixes;

		return *this;
	}

	/**
	*	@return If specified, the override extension, null otherwise.
	*/
	const char* GetOverrideExtension() const { return m_pszOverrideExt; }

	/**
	*	Sets the override extension for the given platform.
	*	@param platform Platform.
	*/
	CLibArgs& OverrideExtension( const char* const pszOverrideExt, const Platform platform )
	{
		if( GetCurrentPlatform() == platform )
			m_pszOverrideExt = pszOverrideExt;

		return *this;
	}

	/**
	*	Sets the override extension.
	*/
	CLibArgs& OverrideExtension( const char* const pszOverrideExt )
	{
		m_pszOverrideExt = pszOverrideExt;

		return *this;
	}

private:
	const char* m_pszFilename;
	const char* m_pszPath = "";
	bool m_bDisablePrefix = false;
	const char* m_pszOverrideExt = nullptr;

private:
	CLibArgs( const CLibArgs& ) = delete;
	CLibArgs& operator=( const CLibArgs& ) = delete;
};

#endif //LIB_CLIBARGS_H