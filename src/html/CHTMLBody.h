#ifndef HTML_CHTMLBODY_H
#define HTML_CHTMLBODY_H

#include "CHTMLComposite.h"

/**
*	Convenience wrapper for the \<body\> element.
*/
class CHTMLBody : public CHTMLComposite
{
public:
	CHTMLBody() = default;
	~CHTMLBody() = default;

	void GenerateHTML( std::stringstream& stream ) override;

private:
	CHTMLBody( const CHTMLBody& ) = delete;
	CHTMLBody& operator=( const CHTMLBody& ) = delete;
};

#endif //HTML_CHTMLBODY_H