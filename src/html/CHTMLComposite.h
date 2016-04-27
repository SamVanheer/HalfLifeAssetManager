#ifndef ANGELSCRIPT_CHTMLCOMPOSITE_H
#define ANGELSCRIPT_CHTMLCOMPOSITE_H

#include <vector>
#include <memory>
#include <string>

#include "IHTMLObject.h"

class CHTMLComposite : public IHTMLObject
{
public:

	CHTMLComposite();
	virtual ~CHTMLComposite();

	virtual void AddObject( std::shared_ptr<IHTMLObject> object );

	virtual void GenerateHTML( std::stringstream& stream );

private:
	std::vector<std::shared_ptr<IHTMLObject>> m_Objects;

private:
	CHTMLComposite( const CHTMLComposite& );
	CHTMLComposite& operator=( const CHTMLComposite& );
};

#endif //ANGELSCRIPT_CHTMLCOMPOSITE_H