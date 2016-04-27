#ifndef ANGELSCRIPT_IHTMLOBJECT_H
#define ANGELSCRIPT_IHTMLOBJECT_H

#include <sstream>

class IHTMLObject
{
public:

	virtual ~IHTMLObject();

	virtual void GenerateHTML( std::stringstream& stream ) = 0;
};

inline IHTMLObject::~IHTMLObject()
{
}

#endif //ANGELSCRIPT_IHTMLOBJECT_H