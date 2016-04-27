#ifndef HTML_CHTMLELEMENT_H
#define HTML_CHTMLELEMENT_H

#include <unordered_map>
#include <string>

#include "CHTMLComposite.h"

/**
*	An HTML element that can have a name, text contents and attributes.
*/
class CHTMLElement : public CHTMLComposite
{
private:
	/**
	*	Object returned by GetAttributeValue if the requested attribute does not exist.
	*/
	static const std::string DEFAULT_ATTRIBUTE_VALUE;

public:

	/**
	*	Creates an element with the given tag name and contents.
	*	@param szTagName Element name.
	*	@param szTextContents Text contents.
	*/
	CHTMLElement( const std::string& szTagName = "", const std::string& szTextContents = "" );
	virtual ~CHTMLElement() = default;

	virtual void GenerateHTML( std::stringstream& stream ) override;

	/**
	*	Gets the element name.
	*/
	const std::string& GetTagName() const { return m_szTagName; }

	/**
	*	Sets the element name.
	*/
	void SetTagName( const std::string& szTagName ) { m_szTagName = szTagName; }

	/**
	*	Gets the text contents.
	*/
	const std::string& GetTextContents() const { return m_szTextContents; }

	/**
	*	Sets the text contents.
	*/
	void SetTextContents( const std::string& szTextContents ) { m_szTextContents = szTextContents; }

	/**
	*	Gets an attribute value by name.
	*/
	const std::string& GetAttributeValue( const std::string& szAttribute );

	/**
	*	Sets an attribute value.
	*/
	void SetAttributeValue( const std::string& szAttribute, const std::string& szValue );

	/**
	*	Clears an attribute from the element.
	*/
	void ClearAttribute( const std::string& szAttribute );

private:
	std::string m_szTagName, m_szTextContents;

	std::unordered_map<std::string, std::string> m_szAttributes;

private:
	CHTMLElement( const CHTMLElement& ) = delete;
	CHTMLElement& operator=( const CHTMLElement& ) = delete;
};

#endif //HTML_CHTMLELEMENT_H