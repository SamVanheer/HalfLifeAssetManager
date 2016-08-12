#include <cassert>

#include "CHTMLComposite.h"

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