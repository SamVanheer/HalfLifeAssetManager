#include "CHTMLPage.h"

#include <cassert>
#include <sstream>

#include "IHTMLObject.h"
#include "CHTMLHeader.h"
#include "CHTMLBody.h"

CHTMLPage::CHTMLPage()
	: m_Header( new CHTMLHeader() )
	, m_Body( new CHTMLBody() )
{
}

std::string CHTMLPage::GenerateHTML()
{
	std::stringstream stream;

	stream << "<html>" << std::endl;
	m_Header->GenerateHTML( stream );
	m_Body->GenerateHTML( stream );
	stream << "</html>" << std::endl;

	return stream.str();
}