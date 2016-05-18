#ifndef LIB_ILIBSYSTEM_H
#define LIB_ILIBSYSTEM_H

#include "LibInterface.h"

/**
*	@ingroup LibInterface
*
*	@{
*/

/**
*	This interface represents a library. When a library is loaded, it connects with other libraries. When it is freed, it disconnects.
*	A library need not implement this interface. However, if it requires a connection to another library, it is the best way to do so.
*/
class ILibSystem : public IBaseInterface
{
public:
	virtual ~ILibSystem() = 0;

	/**
	*	Connects the library with other libraries.
	*	@param appFactory Factory function used to create interfaces to communicate with the main application.
	*	@param fileSystemFactory Factory function used to create filesystems.
	*	@return true on success, false otherwise.
	*/
	virtual bool Connect( const CreateInterfaceFn* const pFactories, const size_t uiNumFactories ) = 0;

	/**
	*	Disconnects the library.
	*/
	virtual void Disconnect() = 0;
};

inline ILibSystem::~ILibSystem()
{
}

/**
*	Interface name.
*/
#define ILIBSYSTEM_NAME "ILibSystemV001"

/** @} */

#endif //LIB_ILIBSYSTEM_H