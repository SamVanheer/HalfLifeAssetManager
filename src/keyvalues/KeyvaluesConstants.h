#ifndef KEYVALUESCONSTANTS_H
#define KEYVALUESCONSTANTS_H

/**
*	Node types.
*	TODO: enum class
*/
enum KeyvalueNodeType
{
	KVNode_Keyvalue,
	KVNode_Block
};

/**
*	Token types used by the lexer.
*	TODO: enum class
*/
enum KeyvalueTokenType
{
	KVToken_None,
	KVToken_BlockOpen,
	KVToken_BlockClose,
	KVToken_Key,			//Also used for block keys
	KVToken_Value
};

/**
*	The control character used for quoted strings.
*/
const char KeyvalueControl_Quote		= '\"';

/**
*	The control character used for block open statements.
*/
const char KeyvalueControl_BlockOpen	= '{';

/**
*	The control character used for block close statements.
*/
const char KeyvalueControl_BlockClose	= '}';

/**
*	Tab width for pretty printing.
*/
const size_t KEYVALUE_TAB_WIDTH = 4;

/**
*	Settings for the lexer.
*/
struct CKeyvaluesLexerSettings final
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