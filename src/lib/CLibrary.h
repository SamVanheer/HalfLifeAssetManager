#ifndef LIB_CLIBRARY_H
#define LIB_CLIBRARY_H

#include <string>

/**
*	Represents a handle to single dynamic/shared library that can be loaded.
*/
class CLibrary final
{
public:
	/**
	*	Type used to represent libraries internally.
	*/
	using LibraryHandle_t = void*;

	/**
	*	Constant that represents a null library handle. Implemented as a function due to compiler limitations (const LibraryHandle_t const triggers a warning).
	*/
	static constexpr LibraryHandle_t NULL_HANDLE()
	{
		return nullptr;
	}

public:
	/**
	*	Constructs an empty library handle.
	*/
	CLibrary();

	/**
	*	Move constructor. Transfers ownership of the given library handle to this library.
	*/
	CLibrary( CLibrary&& other );

	/**
	*	Move assignment operator. Transfers ownership of the given library to this library handle. If this handle currently represents a loaded library, it will be freed.
	*/
	CLibrary& operator=( CLibrary&& other );

	/**
	*	Frees the library if it wasn't already freed before.
	*/
	~CLibrary();

	/**
	*	@return The name of this library.
	*/
	const std::string& GetName() const { return m_szName; }

	/**
	*	Returns the internal handle to the library. This is a platform specific handle.
	*/
	LibraryHandle_t GetLibraryHandle() const { return m_hLibrary; }

	/**
	*	Returns whether the library is loaded or not.
	*/
	bool IsLoaded() const { return m_hLibrary != NULL_HANDLE(); }

	/**
	*	Loads the library with the given name. If this handle already has a handle to a library, it will be freed.
	*	@param pszFilename Name of the library. This includes the path and extension.
	*	@return true if the library was successfully loaded, false otherwise.
	*/
	bool Load( const char* const pszFilename );

	/**
	*	If the library is loaded, this will free the library.
	*/
	void Free();

	/**
	*	Gets a function from the library by name.
	*	@param pszName Name of the function to get.
	*	@return Pointer to the function on success, null otherwise.
	*/
	void* GetFunctionAddress( const char* const pszName ) const;

private:
	/**
	*	Does the actual of loading the library.
	*/
	static LibraryHandle_t DoLoad( const char* const pszFilename );

	/**
	*	Does the actual freeing of the library.
	*/
	static void DoFree( LibraryHandle_t hLibrary );

	/**
	*	Does the actual getting of the address.
	*/
	static void* DoGetFunctionAddress( LibraryHandle_t hLibrary, const char* const pszName );

private:
	std::string m_szName;
	LibraryHandle_t m_hLibrary = NULL_HANDLE();

private:
	CLibrary( const CLibrary& ) = delete;
	CLibrary& operator=( const CLibrary& ) = delete;
};

#endif //LIB_CLIBRARY_H