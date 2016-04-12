#ifndef CKVBLOCKNODE_H
#define CKVBLOCKNODE_H

#include <memory>
#include <vector>
#include <string>

#include "CKeyvalueNode.h"

/*
* A single keyvalue block node
* Blocks have 0 or more child keyvalues
*/
class CKvBlockNode : public CKeyvalueNode
{
public:
	typedef CKeyvalueNode BaseClass;

	typedef std::vector<std::shared_ptr<CKeyvalueNode>> Children_t;

public:
	/*
	* Constructs a keyvalue node with a non-empty key
	* if pszKey is NULL or empty, the key is set to KEYVALUE_DEFAULT_KEY
	*/
	CKvBlockNode( const char* pszKey );

	/*
	* Constructs a keyvalue node with a non-empty key
	* if pszKey is NULL or empty, the key is set to KEYVALUE_DEFAULT_KEY
	* children are set to the given list of children
	*/
	CKvBlockNode( const char* pszKey, const Children_t& children );

	/*
	* Constructs a keyvalue node with a non-empty key
	* if pszKey is NULL or empty, the key is set to KEYVALUE_DEFAULT_KEY
	* The given child is made the first child
	*/
	CKvBlockNode( const char* pszKey, std::shared_ptr<CKeyvalueNode> firstChild );

	const Children_t& GetChildren() const { return m_Children; }
	Children_t& GetChildren() { return m_Children; }

	void SetChildren( const Children_t& children );

	void RemoveAllChildren();

	void RemoveAllNotNamed( const char* pszKey );

	std::shared_ptr<CKeyvalueNode> FindFirstChild( const char* pszKey ) const;

	std::shared_ptr<CKeyvalueNode> FindFirstChild( const char* pszKey, const KeyvalueNodeType type ) const;

	template<typename T>
	std::shared_ptr<T> FindFirstChild( const char* const pszKey ) const;

	CString FindFirstKeyvalue( const char* pszKey ) const;

	void AddKeyvalue( const char* const pszKey, const char* const pszValue );

	virtual void Print( const size_t uiTabLevel = 0 ) const;

	void PrintChildren( const size_t uiTabLevel = 0 ) const;

private:
	Children_t m_Children;

private:
	CKvBlockNode( const CKvBlockNode& ) = delete;
	CKvBlockNode& operator=( const CKvBlockNode& ) = delete;
};

template<typename T>
std::shared_ptr<T> CKvBlockNode::FindFirstChild( const char* const pszKey ) const
{
	auto node = FindFirstChild( pszKey );

	if( !node )
		return nullptr;

	return std::static_pointer_cast<T>( node );
}

#endif //CKVBLOCKNODE_H