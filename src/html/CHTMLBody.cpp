#include "CHTMLBody.h"

void CHTMLBody::GenerateHTML( std::stringstream& stream )
{
	stream << "<body>" << std::endl;
	CHTMLComposite::GenerateHTML( stream );
	stream << "</body>" << std::endl;
}