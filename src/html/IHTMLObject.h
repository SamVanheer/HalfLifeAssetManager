#ifndef HTML_IHTMLOBJECT_H
#define HTML_IHTMLOBJECT_H

#include <sstream>

/**
*	Interface that all HTML classes implement.
*/
class IHTMLObject
{
public:

	virtual ~IHTMLObject();

	/**
	*	Generates HTML and inserts it into the stream.
	*/
	virtual void GenerateHTML( std::stringstream& stream ) = 0;
};

inline IHTMLObject::~IHTMLObject()
{
}

#endif //HTML_IHTMLOBJECT_H