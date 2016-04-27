#ifndef CKEYVALUE_H
#define CKEYVALUE_H

#include "CKeyvalueNode.h"

namespace keyvalues
{
/**
* A single keyvalue
*/
class CKeyvalue final : public CKeyvalueNode
{
public:
	typedef CKeyvalueNode BaseClass;

public:
	/**
	*	Constructs a keyvalue with a key and an optional value.
	*	@param pszKey Key. Must be non-null.
	*	@param pszValue Value. Must be non-null.
	*/
	CKeyvalue( const char* const pszKey, const char* const pszValue = "" );

	/**
	*	Gets the value.
	*/
	const CString& GetValue() const { return m_szValue; }

	/**
	*	Sets the value.
	*	@param pszValue Value. Must be non-null.
	*/
	void SetValue( const char* const pszValue );

	/**
	*	@see SetValue( const char* const pszValue )
	*/
	void SetValue( const CString& szValue );

	//TODO: move
	virtual void Print( const size_t uiTabLevel = 0 ) const override;

private:
	CString m_szValue;

private:
	CKeyvalue( const CKeyvalue& ) = delete;
	CKeyvalue& operator=( const CKeyvalue& ) = delete;
};
}

#endif //CKEYVALUE_H