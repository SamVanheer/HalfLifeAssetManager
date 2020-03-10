#ifndef LIB_LIBCONSTANTS_H
#define LIB_LIBCONSTANTS_H

/**
*	Platform identifiers
*/
enum class Platform
{
	/**
	*	This is an unknown platform.
	*/
	UNKNOWN		= -1,
	WINDOWS		= 0,
	LINUX,
	MAC,
};

/**
*	@return The current platform.
*/
inline Platform GetCurrentPlatform()
{
#ifdef WIN32
	return Platform::WINDOWS;
#elif defined( __linux__ )
	return Platform::LINUX;
#elif defined( __APPLE__ )
	return Platform::MAC;
#else
	return Platform::UNKNOWN;
#endif
}

/**
*	Class that represents a platform.
*/
class CPlatform final
{
public:
	/**
	*	Gets a platform instance for the given platform code.
	*/
	static const CPlatform& GetPlatform( const Platform platform );

	/**
	*	@return The CPlatform instance that represents the current platform.
	*/
	static const CPlatform& GetCurrentPlatform();

	/**
	*	Constructor.
	*	@param platform Platform code.
	*	@param pszName Name of this platform.
	*	@param pszDefaultLibPrefix Default library prefix.
	*	@param pszDefaultLibExt Default library extension.
	*	@param pszLibExts List of common library extensions.
	*	@param uiNumLibExts Number of common library extensions.
	*/
	CPlatform( 
		const Platform platform,
		const char* const pszName, 
		const char* const pszDefaultLibPrefix, const char* const pszDefaultLibExt,
		const char* const* const pszLibExts, const size_t uiNumLibExts );

	/**
	*	@return The Platform code for this platform.
	*	@see Platform
	*/
	Platform GetPlatformCode() const { return m_Platform; }

	/**
	*	@return The name of this platform.
	*/
	const char* GetName() const { return m_pszName; }

	/**
	*	@return The default library prefix.
	*/
	const char* GetDefaultLibPrefix() const { return m_pszDefaultLibPrefix; }

	/**
	*	@return The default library extension for the platform.
	*/
	const char* GetDefaultLibExtension() const { return m_pszDefaultLibExt; }

	/**
	*	@return The list of common library extensions.
	*/
	const char* const* const GetLibExts() const { return m_pszLibExts; }

	/**
	*	Gets a common library extension by index.
	*/
	const char* GetLibExt( const size_t uiIndex ) const;

	/**
	*	@return The number of common library extensions.
	*/
	size_t GetNumLibExts() const { return m_uiNumLibExts; }

private:
	const Platform m_Platform;
	const char* const m_pszName;
	const char* const m_pszDefaultLibPrefix;
	const char* const m_pszDefaultLibExt;

	const char* const* const m_pszLibExts;
	const size_t m_uiNumLibExts;

private:
	CPlatform( const CPlatform& ) = delete;
	CPlatform& operator=( const CPlatform& ) = delete;
};

#endif //LIB_LIBCONSTANTS_H