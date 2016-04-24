#ifndef KEYVALUES_KVFORWARD_H
#define KEYVALUES_KVFORWARD_H

/**
*	Forward declarations for keyvalues types.
*/

namespace keyvalues
{
class CKeyvalueNode;
class CKeyvalue;
class CKeyvalueBlock;
class CKeyvaluesLexer;
class CKeyvaluesParser;
class CIterativeKeyvaluesParser;
class CKeyvaluesWriter;

//Define shorthand notation for common types.
typedef CKeyvalueNode				Node;
typedef CKeyvalue					KV;
typedef CKeyvalueBlock				Block;
typedef CKeyvaluesParser			Parser;
typedef CIterativeKeyvaluesParser	IterativeParser;
typedef CKeyvaluesWriter			Writer;
}

//Define a shorter namespace.
namespace kv = keyvalues;

#endif //KEYVALUES_KVFORWARD_H