#ifndef ANGELSCRIPT_CHTMLCOMPOSITE_H
#define ANGELSCRIPT_CHTMLCOMPOSITE_H

#include <vector>
#include <memory>
#include <string>

#include "IHTMLObject.h"

/**
*	An HTML element that can have multiple HTML elements as children.
*/
class CHTMLComposite : public IHTMLObject
{
public:

	CHTMLComposite() = default;
	virtual ~CHTMLComposite() = default;

	/**
	*	Adds an element to this element.
	*	@param object Element to add. Must be non-null.
	*/
	virtual void AddObject( std::shared_ptr<IHTMLObject> object );

	virtual void GenerateHTML( std::stringstream& stream ) override;

private:
	std::vector<std::shared_ptr<IHTMLObject>> m_Objects;

private:
	CHTMLComposite( const CHTMLComposite& ) = delete;
	CHTMLComposite& operator=( const CHTMLComposite& ) = delete;
};

#endif //ANGELSCRIPT_CHTMLCOMPOSITE_H