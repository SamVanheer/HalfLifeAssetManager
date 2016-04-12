#ifndef CKEYVALUE_H
#define CKEYVALUE_H

#include "CKeyvalueNode.h"

/*
* A single keyvalue
*/
class CKeyvalue : public CKeyvalueNode
{
public:
	typedef CKeyvalueNode BaseClass;

public:
	/*
	* Constructs an empty keyvalue
	* the key is set to KEYVALUE_DEFAULT_KEY
	*/
	CKeyvalue();

	/*
	* Constructs a keyvalue with a non-empty key and an optional value
	* if pszKey is null or empty, the key is set to KEYVALUE_DEFAULT_KEY
	*/
	CKeyvalue( const char* pszKey, const char* pszValue = nullptr );

	const CString& GetValue() const { return m_szValue; }

	void SetValue( const char* pszValue );
	void SetValue( const CString& szValue );

	virtual void Print( const size_t uiTabLevel = 0 ) const;

private:
	CString m_szValue;

private:
	CKeyvalue( const CKeyvalue& ) = delete;
	CKeyvalue& operator=( const CKeyvalue& ) = delete;
};

#endif //CKEYVALUE_H