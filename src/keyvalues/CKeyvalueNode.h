#ifndef CKEYVALUENODE_H
#define CKEYVALUENODE_H

#include <cstdlib>

#include "utility/CString.h"

#include "KeyvaluesConstants.h"

/*
* A single keyvalue node
* The node type is decided on object construction, and cannot be changed afterwards
*/
class CKeyvalueNode
{
public:
	/*
	* Constructs a keyvalue node with a non-empty key
	* if pszKey is NULL or empty, the key is set to KEYVALUE_DEFAULT_KEY
	*/
	CKeyvalueNode( const char* pszKey, const KeyvalueNodeType type );
	CKeyvalueNode( const KeyvalueNodeType type );

	virtual ~CKeyvalueNode() {}

	const CString& GetKey() const { return m_szKey; }

	void SetKey( const char* pszKey );
	void SetKey( const CString& szKey );

	KeyvalueNodeType GetType() const { return m_Type; }

	virtual void Print( const size_t uiTabLevel = 0 ) const = 0;

private:

private:
	CString m_szKey;
	const KeyvalueNodeType m_Type;

private:
	CKeyvalueNode( const CKeyvalueNode& ) = delete;
	CKeyvalueNode& operator=( const CKeyvalueNode& ) = delete;
};

#endif //CKEYVALUENODE_H