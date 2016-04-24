#ifndef CKEYVALUENODE_H
#define CKEYVALUENODE_H

#include <cstdlib>

#include "utility/CString.h"

#include "KeyvaluesConstants.h"

/**
*	A single keyvalue node
*/
class CKeyvalueNode
{
public:
	/**
	*	Constructs a keyvalue node with a key.
	*	@
	*/
	CKeyvalueNode( const char* const pszKey, const KeyvalueNodeType type );

	virtual ~CKeyvalueNode() {}

	const CString& GetKey() const { return m_szKey; }

	/**
	*	Sets the node key. Must be non-null.
	*/
	void SetKey( const char* const pszKey );

	/**
	*	@see SetKey( const char* const pszKey )
	*/
	void SetKey( const CString& szKey );

	/**
	*	Gets the node type.
	*/
	KeyvalueNodeType GetType() const { return m_Type; }

	//TODO: move this out of the class
	virtual void Print( const size_t uiTabLevel = 0 ) const = 0;

private:
	CString m_szKey;
	const KeyvalueNodeType m_Type;

private:
	CKeyvalueNode( const CKeyvalueNode& ) = delete;
	CKeyvalueNode& operator=( const CKeyvalueNode& ) = delete;
};

#endif //CKEYVALUENODE_H