#ifndef CKEYVALUES_H
#define CKEYVALUES_H

#include "CKeyvalue.h"
#include "CKvBlockNode.h"

/*
* Keyvalues framework:
* This framework supports the reading of keyvalues text files
*/

/*
* A collection of keyvalues
*/
class CKeyvalues
{
public:
	/*
	* Constructs an empty collection
	*/
	CKeyvalues();

	/*
	* Constructs a collection of keyvalues with the given block as its root
	*/
	CKeyvalues( std::shared_ptr<CKvBlockNode> rootBlock );

	std::shared_ptr<const CKvBlockNode> GetRoot() const { return m_RootBlock; }
	std::shared_ptr<CKvBlockNode> GetRoot() { return m_RootBlock; }

	void SetRoot( std::shared_ptr<CKvBlockNode> rootBlock );

	void Reset();

private:
	std::shared_ptr<CKvBlockNode> m_RootBlock;

private:
	CKeyvalues( const CKeyvalues& );
	CKeyvalues& operator=( const CKeyvalues& );
};

#endif //CKEYVALUES_H