#include "CHTMLComposite.h"

#include <cassert>
#include <sstream>

#include "IHTMLObject.h"

CHTMLComposite::CHTMLComposite()
{
}

CHTMLComposite::~CHTMLComposite()
{
}

void CHTMLComposite::AddObject( std::shared_ptr<IHTMLObject> object )
{
	assert( object );

	m_Objects.push_back( object );
}

void CHTMLComposite::GenerateHTML( std::stringstream& stream )
{
	for( auto& pObject : m_Objects )
	{
		pObject->GenerateHTML( stream );
	}
}