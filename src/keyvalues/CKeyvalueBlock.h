#ifndef KEYVALUES_CKEYVALUEBLOCK_H
#define KEYVALUES_CKEYVALUEBLOCK_H

#include <vector>

#include "CKeyvalueNode.h"

namespace keyvalues
{
/**
*	A single keyvalue block node
*	Blocks have 0 or more child keyvalues
*/
class CKeyvalueBlock final : public CKeyvalueNode
{
public:
	typedef CKeyvalueNode BaseClass;

	typedef std::vector<CKeyvalueNode*> Children_t;

public:
	/*
	*	Constructs a keyvalue node with a key.
	*	@param pszKey Key. Must be non-null.
	*/
	CKeyvalueBlock( const char* const pszKey );

	/**
	*	Constructs a keyvalue node with a key.
	*	Children are set to the given list of children.
	*	@param pszKey Key. Must be non-null.
	*	@param children Children to add.
	*/
	CKeyvalueBlock( const char* const pszKey, const Children_t& children );

	/**
	*	Constructs a keyvalue node with a key.
	*	The given child is made the first child.
	*	@param pszKey Key. Must be non-null.
	*	@param pFirstChild First child. Must be non-null.
	*/
	CKeyvalueBlock( const char* pszKey, CKeyvalueNode* pFirstChild );

	~CKeyvalueBlock();

	const Children_t& GetChildren() const { return m_Children; }
	//TODO: remove this and add ways to add/remove children safely.
	Children_t& GetChildren() { return m_Children; }

	/**
	*	Gets a list of children that have the given key. The children are still managed by this block.
	*	@param pszKey Key. Must be non-null.
	*	@return List of zero or more children matching the given key.
	*/
	Children_t GetChildrenByKey( const char* const pszKey ) const;

	/**
	*	Sets the children to the given list. Any nodes that were previously children of this node are destroyed.
	*	There may not be any null children in the list.
	*/
	void SetChildren( const Children_t& children );

	/**
	*	Removes all children. The children are destroyed.
	*/
	void RemoveAllChildren();

	/**
	*	Removes all nodes not named pszKey.
	*	@param pszKey Key. Must be non-null.
	*/
	void RemoveAllNotNamed( const char* const pszKey );

	/**
	*	Finds the first child with the given key.
	*	@param pszKey Key. Must be non-null.
	*	@return If found, the first child node with the given key, null otherwise.
	*/
	CKeyvalueNode* FindFirstChild( const char* const pszKey ) const;

	/**
	*	Finds the first child with the given key, and that has the given type.
	*	@param pszKey Key. Must be non-null.
	*	@param type Node type to filter by.
	*	@return If found, the first child node with the given key and type, null otherwise.
	*/
	CKeyvalueNode* FindFirstChild( const char* const pszKey, const NodeType type ) const;

	/**
	*	Finds the first child with the given key, and that has the given class type.
	*	@param pszKey Key. Must be non-null.
	*	@tparam T Class type to filter by.
	*	@return If found, the first child node with the given key and class type, null otherwise.
	*/
	template<typename T>
	T* FindFirstChild( const char* const pszKey ) const;

	/**
	*	Finds the first value associated with the given key.
	*	@param pszKey Key. Must be non-null.
	*	@return If found, the value. Otherwise, an empty string.
	*/
	CString FindFirstKeyvalue( const char* pszKey ) const;

	/**
	*	Adds a keyvalue.
	*	@param pszKey Key. Must be non-null.
	*	@param pszValue Value. Must be non-null. Maybe an empty string.
	*/
	void AddKeyvalue( const char* const pszKey, const char* const pszValue );

	//TODO: move
	virtual void Print( const size_t uiTabLevel = 0 ) const override;

	void PrintChildren( const size_t uiTabLevel = 0 ) const;

private:
	Children_t m_Children;

private:
	CKeyvalueBlock( const CKeyvalueBlock& ) = delete;
	CKeyvalueBlock& operator=( const CKeyvalueBlock& ) = delete;
};

template<typename T>
T* CKeyvalueBlock::FindFirstChild( const char* const pszKey ) const
{
	auto node = FindFirstChild( pszKey );

	if( !node )
		return nullptr;

	//TODO: add static const type to the node subclasses, use that to determine the type.

	return dynamic_cast<T*>( node );
}
}

#endif //KEYVALUES_CKEYVALUEBLOCK_H
