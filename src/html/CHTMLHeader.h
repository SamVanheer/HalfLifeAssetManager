#ifndef ANGELSCRIPT_CHTMLHEADER_H
#define ANGELSCRIPT_CHTMLHEADER_H

#include <memory>

#include "IHTMLObject.h"

class CHTMLElement;

class CHTMLHeader : public IHTMLObject
{
public:

	CHTMLHeader();
	~CHTMLHeader();

	virtual void GenerateHTML( std::stringstream& stream );

	std::shared_ptr<CHTMLElement> GetTitle() const { return m_Title; }

	std::shared_ptr<CHTMLElement> GetStyleSheet() const { return m_StyleSheet; }

private:
	std::shared_ptr<CHTMLElement> m_Title, m_StyleSheet;

private:

	CHTMLHeader( const CHTMLHeader& );
	CHTMLHeader& operator=( const CHTMLHeader& );
};

#endif //ANGELSCRIPT_CHTMLHEADER_H