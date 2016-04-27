#include "CHTMLHeader.h"

#include <sstream>

#include "CHTMLElement.h"

CHTMLHeader::CHTMLHeader()
	: m_Title( new CHTMLElement( "title" ) )
	, m_StyleSheet( new CHTMLElement( "link" ) )
{
	m_StyleSheet->SetAttributeValue( "rel", "stylesheet" );
	m_StyleSheet->SetAttributeValue( "type", "text/css" );
}

CHTMLHeader::~CHTMLHeader()
{
}

void CHTMLHeader::GenerateHTML( std::stringstream& stream )
{
	stream << "<head>" << std::endl << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\">" << std::endl;

	if( !m_StyleSheet->GetAttributeValue( "href" ).empty() )
	{
		m_StyleSheet->GenerateHTML( stream );
	}

	m_Title->GenerateHTML( stream );

	stream << "</head>" << std::endl;
}