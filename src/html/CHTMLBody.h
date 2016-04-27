#ifndef ANGELSCRIPT_CHTMLBODY_H
#define ANGELSCRIPT_CHTMLBODY_H

#include "CHTMLComposite.h"

class CHTMLBody : public CHTMLComposite
{
public:
	CHTMLBody();
	~CHTMLBody();

	void GenerateHTML( std::stringstream& stream );

private:
	CHTMLBody( const CHTMLBody& );
	CHTMLBody& operator=( const CHTMLBody& );
};

#endif //ANGELSCRIPT_CHTMLBODY_H