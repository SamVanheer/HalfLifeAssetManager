#include "CHTMLElement.h"

const std::string CHTMLElement::DEFAULT_ATTRIBUTE_VALUE = "";

CHTMLElement::CHTMLElement( const std::string& szTagName, const std::string& szTextContents )
	: m_szTagName( szTagName )
{
	SetTextContents( szTextContents );
}

void CHTMLElement::GenerateHTML( std::stringstream& stream )
{
	stream << '<' << m_szTagName;

	if( !m_szAttributes.empty() )
	{
		for( auto& attr : m_szAttributes )
		{
			stream << ' ' << attr.first << "=\"" << attr.second << '\"';
		}
	}

	stream << '>';

	CHTMLComposite::GenerateHTML( stream );
	
	stream << m_szTextContents << "</" << m_szTagName << '>' << std::endl;
}

const std::string& CHTMLElement::GetAttributeValue( const std::string& szAttribute )
{
	auto it = m_szAttributes.find( szAttribute );

	return ( it != m_szAttributes.end() ? it->second : DEFAULT_ATTRIBUTE_VALUE );
}

void CHTMLElement::SetAttributeValue( const std::string& szAttribute, const std::string& szValue )
{
	m_szAttributes.insert( std::make_pair( szAttribute, szValue ) );
}

void CHTMLElement::ClearAttribute( const std::string& szAttribute )
{
	auto it = m_szAttributes.find( szAttribute );

	if( it != m_szAttributes.end() )
		m_szAttributes.erase( it );
}