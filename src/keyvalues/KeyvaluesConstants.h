#ifndef KEYVALUESCONSTANTS_H
#define KEYVALUESCONSTANTS_H

/*
* The default key assigned to a keyvalue node if none has been set, or an invalid key was set
*/
#define KEYVALUE_DEFAULT_KEY "INVALID_KEY"

/*
* The name that the root node has
*/
#define KEYVALUE_ROOT_NODE_NAME "ROOT_NODE"

#define KEYVALUE_UNNAMED_BLOCK "UNNAMED"

enum KeyvalueNodeType
{
	KVNode_Keyvalue,
	KVNode_Block
};

enum KeyvalueTokenType
{
	KVToken_None,
	KVToken_BlockOpen,
	KVToken_BlockClose,
	KVToken_Key,			//Also used for block keys
	KVToken_Value
};

const char KeyvalueControl_Quote		= '\"';
const char KeyvalueControl_BlockOpen	= '{';
const char KeyvalueControl_BlockClose	= '}';

//For pretty printing
const size_t KEYVALUE_TAB_WIDTH = 4;

struct CKeyvaluesLexerSettings
{
	bool fAllowUnnamedBlocks;
	bool fLogErrors;
	bool fLogWarnings;

	CKeyvaluesLexerSettings()
		: fAllowUnnamedBlocks( false )
		, fLogErrors( true )
		, fLogWarnings( true )
	{
	}
};

#endif //KEYVALUESCONSTANTS_H