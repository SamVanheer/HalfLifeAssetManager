#ifndef CKEYVALUENODE_H
#define CKEYVALUENODE_H

#include <cstdlib>

#include "utility/CString.h"

#include "KeyvaluesConstants.h"

namespace keyvalues
{
/**
*	A single keyvalue node
*/
class CKeyvalueNode
{
public:
	/**
	*	Constructs a keyvalue node with a key.
	*	@param pszKey Key. Must be non-null.
	*	@param type Node type.
	*/
	CKeyvalueNode( const char* const pszKey, const NodeType type );

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
	NodeType GetType() const { return m_Type; }

	//TODO: move this out of the class
	virtual void Print( const size_t uiTabLevel = 0 ) const = 0;

private:
	CString m_szKey;
	const NodeType m_Type;

private:
	CKeyvalueNode( const CKeyvalueNode& ) = delete;
	CKeyvalueNode& operator=( const CKeyvalueNode& ) = delete;
};
}

#endif //CKEYVALUENODE_H