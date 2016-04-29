#ifndef LIB_LIBINTERFACE_H
#define LIB_LIBINTERFACE_H

#include <type_traits>

#include "shared/Platform.h"

/** @file */

/**
*	@defgroup LibInterface Library interface definitions.
*
*	@{
*/

/**
*	Base interface for all interfaces.
*/
class IBaseInterface
{
public:
	virtual ~IBaseInterface() = 0;
};

inline IBaseInterface::~IBaseInterface()
{
}

/**
*	Function that creates new instances of an interface.
*/
using InstantiateInterfaceFn = IBaseInterface* ( * )();

/**
*	Per-library registry of interface instantiation functions.
*/
class CInterfaceRegistry final
{
public:
	CInterfaceRegistry( const char* const pszName, const InstantiateInterfaceFn instantiateFn );

	static CInterfaceRegistry* GetHead() { return m_pHead; }
	CInterfaceRegistry* GetNext() const { return m_pNext; }

	const char* GetName() const { return m_pszName; }

	InstantiateInterfaceFn GetInstantiateFn() const { return m_InstantiateFn; }

private:
	static CInterfaceRegistry* m_pHead;
	CInterfaceRegistry* m_pNext;

	const char* const m_pszName;
	const InstantiateInterfaceFn m_InstantiateFn;

private:
	CInterfaceRegistry( const CInterfaceRegistry& ) = delete;
	CInterfaceRegistry& operator=( const CInterfaceRegistry& ) = delete;
};

/**
*	Result codes for interface creation.
*/
enum class IFaceResult
{
	SUCCESS = 0,
	FAILURE
};

/**
*	Creates instances of interfaces. This function is implemented in each library.
*	@param pszName Name of the interface to instantiate.
*	@param pResult If non-null, this will be set to the result of the creation. In the event that an interface instantiation returns null, this can be used to detect whether it found the interface or not.
*	@return Pointer to the interface, or null if the interface couldn't be found or created.
*/
extern "C" DLLEXPORT IBaseInterface* CreateInterface( const char* const pszName, IFaceResult* pResult = nullptr );

/**
*	Name of the CreateInterface function.
*/
#define CREATEINTERFACE_NAME "CreateInterface"

/**
*	Function pointer that can store CreateInterface.
*/
using CreateInterfaceFn = IBaseInterface* ( * )( const char* const pszName, IFaceResult* pResult );

/**
*	Registers an interface instantiation function in the library's list of interfaces.
*	@param pszName Name of the interface.
*	@param className Name of the concrete class that implements this interface.
*	@param function The function used to create the interface. This must be a function that uses InstantiateInterfaceFn's format.
*	@see InstantiateInterfaceFn
*/
#define REGISTER_INTERFACE_FUNCTION( pszName, className, function )			\
static CInterfaceRegistry __g_##className##Reg__( pszName, function )

/**
*	Registers a global variable in the library's list of interfaces.
*	@param pszName Name of the interface.
*	@param className Name of the concrete class that implements the interface.
*	@param pGlobal Pointer to the global variable to register.
*/
#define REGISTER_INTERFACE_GLOBAL( pszName, className, pGlobal )					\
static IBaseInterface* __Create##className##Fn()									\
{																					\
	return static_cast<IBaseInterface*>( pGlobal );									\
}																					\
																					\
REGISTER_INTERFACE_FUNCTION( pszName, className, __Create##className##Fn )

/**
*	Registers a class in the library's list of interfaces. The class will be instanced every time the interface is requested.
*	@param pszName Name of the interface.
*	@param className Name of the concrete class that implements the interface.
*/
#define REGISTER_INTERFACE( pszName, className )									\
static IBaseInterface* __Create##className##Fn()									\
{																					\
	return static_cast<IBaseInterface*>( new className() );							\
}																					\
																					\
REGISTER_INTERFACE_FUNCTION( pszName, className, __Create##className##Fn )

/**
*	Registers a class in the library's list of interfaces. Only one instance of the class will exist.
*	@param pszName Name of the interface.
*	@param className Name of the concrete class that implements the interface.
*/
#define REGISTER_SINGLE_INTERFACE( pszName, className )								\
static className __g_##className##Instance;											\
																					\
REGISTER_INTERFACE_GLOBAL( pszName, className, &__g_##className##Instance )

/** @} */

#endif //LIB_LIBINTERFACE_H