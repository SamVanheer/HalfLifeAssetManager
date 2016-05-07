#ifndef CVAR_CCVAR_H
#define CVAR_CCVAR_H

#include <cassert>

#include "CBaseConCommand.h"

/**
*	@ingroup CVar
*	@{
*/
namespace cvar
{
class CCVar;

/**
*	Callback for cvar changes.
*/
using CVarCallback = void ( * )( CCVar& cvar, const char* pszOldValue, float flOldValue );

/**
*	Interface for objects that want to be informed of cvar changes.
*/
class ICVarHandler
{
public:
	virtual ~ICVarHandler() = 0;

	/**
	*	Called when a cvar has changed.
	*	@param cvar CVar that was changed.
	*	@param pszOldValue Old string value.
	*	@param flOldValue Old float value.
	*/
	virtual void HandleCVar( CCVar& cvar, const char* pszOldValue, float flOldValue ) = 0;
};

inline ICVarHandler::~ICVarHandler()
{
}

/**
*	Builder for CVar arguments.
*/
class CCVarArgsBuilder final
{
public:
	CCVarArgsBuilder() = default;

	const char* GetHelpInfo() const { return m_pszHelpInfo; }

	CCVarArgsBuilder& HelpInfo( const char* const pszHelpInfo );

	Flags_t GetFlags() const { return m_Flags; }

	CCVarArgsBuilder& Flags( const Flags_t flags );

	const char* GetStringValue() const { return m_pszValue; }

	CCVarArgsBuilder& StringValue( const char* const pszValue );

	float GetFloatValue() const { return m_flValue; }

	CCVarArgsBuilder& FloatValue( const float flValue );

	bool UsingStringValue() const { return m_bUsingStringValue; }

	float GetMinValue() const { return m_flMinValue; }

	CCVarArgsBuilder& MinValue( const float flMinValue );

	bool HasMinValue() const { return m_bHasMinValue; }

	float GetMaxValue() const { return m_flMaxValue; }

	CCVarArgsBuilder& MaxValue( const float flMaxValue );

	bool HasMaxValue() const { return m_bHasMaxValue; }

	/**
	*	Gets the callback type.
	*/
	CallbackType GetCallbackType() const { return m_CallbackType; }

	/**
	*	Gets the callback as a function. Only valid if GetCallbackType == CallbackType::FUNCTION.
	*/
	CVarCallback GetCallbackFn() const { return m_CallbackFn; }

	/**
	*	Sets the callback as a function.
	*	@param callbackFn Function to set. Can be null.
	*/
	CCVarArgsBuilder& Callback( const CVarCallback callbackFn );

	/**
	*	Gets the callback as an object. Only valid if GetCallbackType == CallbackType::INTERFACE.
	*/
	ICVarHandler* GetHandler() const { return m_pCallbackObj; }

	/**
	*	Sets the callback as an object.
	*	@param pCallbackObj Object to set. Can be null.
	*/
	CCVarArgsBuilder& Handler( ICVarHandler* pCallbackObj );

private:
	const char*		m_pszHelpInfo		= "";
	Flags_t			m_Flags				= Flag::NONE;

	const char*		m_pszValue			= "";
	float			m_flValue			= 0;

	bool			m_bUsingStringValue	= true;

	float			m_flMinValue		= 0;
	float			m_flMaxValue		= 0;

	bool			m_bHasMinValue		= false;
	bool			m_bHasMaxValue		= false;

	union
	{
		CVarCallback	m_CallbackFn	= nullptr;
		ICVarHandler*	m_pCallbackObj;
	};

	CallbackType m_CallbackType = CallbackType::FUNCTION;

private:
	CCVarArgsBuilder( const CCVarArgsBuilder& ) = delete;
	CCVarArgsBuilder& operator=( const CCVarArgsBuilder& ) = delete;
};

inline CCVarArgsBuilder& CCVarArgsBuilder::HelpInfo( const char* const pszHelpInfo )
{
	m_pszHelpInfo = pszHelpInfo;

	return *this;
}

inline CCVarArgsBuilder& CCVarArgsBuilder::Flags( const Flags_t flags )
{
	m_Flags = flags;

	return *this;
}

inline CCVarArgsBuilder& CCVarArgsBuilder::StringValue( const char* const pszValue )
{
	m_pszValue = pszValue;

	m_bUsingStringValue = true;

	return *this;
}

inline CCVarArgsBuilder& CCVarArgsBuilder::FloatValue( const float flValue )
{
	m_flValue = flValue;

	m_bUsingStringValue = false;

	return *this;
}

inline CCVarArgsBuilder& CCVarArgsBuilder::MinValue( const float flMinValue )
{
	m_flMinValue = flMinValue;

	m_bHasMinValue = true;

	return *this;
}

inline CCVarArgsBuilder& CCVarArgsBuilder::MaxValue( const float flMaxValue )
{
	m_flMaxValue = flMaxValue;

	m_bHasMaxValue = true;

	return *this;
}

inline CCVarArgsBuilder& CCVarArgsBuilder::Callback( const CVarCallback callbackFn )
{
	m_CallbackFn = callbackFn;

	m_CallbackType = CallbackType::FUNCTION;

	return *this;
}

inline CCVarArgsBuilder& CCVarArgsBuilder::Handler( ICVarHandler* pCallbackObj )
{
	m_pCallbackObj = pCallbackObj;

	m_CallbackType = CallbackType::INTERFACE;

	return *this;
}

class CCVar : public CBaseConCommand
{
public:
	/**
	*	Creates a cvar with the given name and arguments.
	*	@param pszName CVar name.
	*	@param args Arguments.
	*	@see CBaseConCommand::CBaseConCommand( const char* const pszName, const char* const pszHelpInfo )
	*/
	CCVar( const char* const pszName, const CCVarArgsBuilder& args = CCVarArgsBuilder() );
	virtual ~CCVar();

	virtual CommandType GetType() const override { return CommandType::CVAR; }

	/**
	*	Gets the callback type.
	*/
	CallbackType GetCallbackType() const { return m_CallbackType; }

	/**
	*	Gets the callback as a function. Only valid if GetCallbackType == CallbackType::FUNCTION.
	*/
	CVarCallback GetCallbackFn() const { return m_CallbackFn; }

	/**
	*	Sets the callback as a function.
	*	@param callbackFn Function to set. Can be null.
	*/
	void SetCallbackFn( const CVarCallback callbackFn );

	/**
	*	Gets the callback as an object. Only valid if GetCallbackType == CallbackType::INTERFACE.
	*/
	ICVarHandler* GetHandler() const { return m_pCallbackObj; }

	/**
	*	Sets the callback as an object.
	*	@param pCallbackObj Object to set. Can be null.
	*/
	void SetHandler( ICVarHandler* pCallbackObj );

	/**
	*	Gets the value as a string.
	*/
	const char* GetString() const	{ return m_pszValue; }

	/**
	*	Gets the value as a float.
	*/
	float		GetFloat() const	{ return m_flValue; }

	/**
	*	Gets the value as an int.
	*/
	int			GetInt() const		{ return static_cast<int>( m_flValue ); }

	/**
	*	Gets the value as a boolean.
	*/
	bool		GetBool() const		{ return m_flValue != 0; }

	/**
	*	Sets the value as a string.
	*/
	void SetString( const char* const pszValue );

	/**
	*	Sets the value as a float.
	*/
	void SetFloat( float flValue );

	/**
	*	Sets the value as an int. Conversion to float is performed.
	*/
	void SetInt( int iValue );

	/**
	*	Sets the value as an int. Conversion to int is performed.
	*/
	void SetBool( const bool bValue );

	/**
	*	Gets the minimum value. Only valid if HasMinValue returns true.
	*	@see HasMinValue
	*/
	float GetMinValue() const { return m_flMinValue; }

	/**
	*	Returns whether a minimum value has been set.
	*/
	bool HasMinValue() const { return m_bHasMinValue; }

	/**
	*	Gets the maximum value. Only valid if HasMaxValue returns true.
	*	@see HasMaxValue
	*/
	float GetMaxValue() const { return m_flMaxValue; }

	/**
	*	Returns whether a maximum value has been set.
	*/
	bool HasMaxValue() const { return m_bHasMaxValue; }

private:
	void Clamp( float& flValue );

	void SetFloatValue( float flValue, const bool bFormatFloat );

	void ValueChanged( const char* pszOldValue, const float flOldValue );

private:

	union
	{
		CVarCallback	m_CallbackFn	= nullptr;
		ICVarHandler*	m_pCallbackObj;
	};

	CallbackType m_CallbackType = CallbackType::FUNCTION;

	char*	m_pszValue			= nullptr;
	float	m_flValue			= 0;

	float	m_flMinValue;
	float	m_flMaxValue;

	bool	m_bHasMinValue;
	bool	m_bHasMaxValue;

private:
	CCVar( const CCVar& ) = delete;
	CCVar& operator=( const CCVar& ) = delete;
};

inline void CCVar::SetCallbackFn( const CVarCallback callbackFn )
{
	m_CallbackFn = callbackFn;

	m_CallbackType = CallbackType::FUNCTION;
}

inline void CCVar::SetHandler( ICVarHandler* pCallbackObj )
{
	m_pCallbackObj = pCallbackObj;

	m_CallbackType = CallbackType::INTERFACE;
}

inline void CCVar::SetFloat( float flValue )
{
	SetFloatValue( flValue, true );
}

inline void CCVar::SetInt( int iValue )
{
	SetFloatValue( static_cast<float>( iValue ), false );
}

inline void CCVar::SetBool( const bool bValue )
{
	SetInt( bValue ? 1 : 0 );
}
}

/**@}*/

#endif //CVAR_CCVAR_H
