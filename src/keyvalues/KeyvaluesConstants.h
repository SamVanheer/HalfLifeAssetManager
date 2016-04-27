#ifndef KEYVALUES_KEYVALUESCONSTANTS_H
#define KEYVALUES_KEYVALUESCONSTANTS_H

namespace keyvalues
{
/**
*	Node types.
*/
enum class NodeType
{
	KEYVALUE,
	BLOCK
};

/**
*	Token types used by the lexer.
*/
enum class TokenType
{
	NONE,
	BLOCK_OPEN,
	BLOCK_CLOSE,
	KEY,			//Also used for block keys
	VALUE
};

/**
*	The control character used for quoted strings.
*/
const char CONTROL_QUOTE		= '\"';

/**
*	The control character used for block open statements.
*/
const char CONTROL_BLOCK_OPEN	= '{';

/**
*	The control character used for block close statements.
*/
const char CONTROL_BLOCK_CLOSE	= '}';

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
}

#endif //KEYVALUES_KEYVALUESCONSTANTS_H
