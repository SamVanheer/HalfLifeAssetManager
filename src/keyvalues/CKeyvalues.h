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
	CKeyvalues( CKvBlockNode* pRootBlock );

	~CKeyvalues();

	const CKvBlockNode* GetRoot() const { return m_pRootBlock; }
	CKvBlockNode* GetRoot() { return m_pRootBlock; }

	void SetRoot( CKvBlockNode* pRootBlock );

	void Reset();

private:
	CKvBlockNode* m_pRootBlock = nullptr;

private:
	CKeyvalues( const CKeyvalues& );
	CKeyvalues& operator=( const CKeyvalues& );
};

#endif //CKEYVALUES_H