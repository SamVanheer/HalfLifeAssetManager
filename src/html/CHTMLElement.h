#ifndef ANGELSCRIPT_CHTMLELEMENT_H
#define ANGELSCRIPT_CHTMLELEMENT_H

#include <unordered_map>
#include <string>

#include "CHTMLComposite.h"

class CHTMLElement : public CHTMLComposite
{
private:
	static const std::string DEFAULT_ATTRIBUTE_VALUE;
public:

	CHTMLElement( const std::string& szTagName = "", const std::string& szTextContents = "" );
	virtual ~CHTMLElement();

	virtual void GenerateHTML( std::stringstream& stream );

	const std::string& GetTagName() const { return m_szTagName; }
	void SetTagName( const std::string& szTagName ) { m_szTagName = szTagName; }

	const std::string& GetTextContents() const { return m_szTextContents; }
	void SetTextContents( const std::string& szTextContents ) { m_szTextContents = szTextContents; }

	const std::string& GetAttributeValue( const std::string& szAttribute );
	void SetAttributeValue( const std::string& szAttribute, const std::string& szValue );
	void ClearAttribute( const std::string& szAttribute );

private:
	std::string m_szTagName, m_szTextContents;

	std::unordered_map<std::string, std::string> m_szAttributes;

private:
	CHTMLElement( const CHTMLElement& );
	CHTMLElement& operator=( const CHTMLElement& );
};

#endif //ANGELSCRIPT_CHTMLELEMENT_H