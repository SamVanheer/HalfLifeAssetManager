#ifndef CGRIDSIZERBUILDER_H
#define CGRIDSIZERBUILDER_H

#include <vector>

#include "ui/wxInclude.h"

#include <wx/sizer.h>

class CGridSizerBuilder final
{
private:
	typedef std::vector<wxWindow*> WindowList_t;

public:
	CGridSizerBuilder( const size_t uiRows, const size_t uiCols );

	bool Add( wxWindow* pWindow, const size_t uiRow, const size_t uiCol );

	wxFlexGridSizer* CreateSizer( const int vgap, const int hgap );

private:
	size_t m_uiRows;
	size_t m_uiCols;

	WindowList_t m_List;
};

inline CGridSizerBuilder::CGridSizerBuilder( const size_t uiRows, const size_t uiCols )
	: m_uiRows( uiRows )
	, m_uiCols( uiCols )
{
	m_List.resize( m_uiRows * m_uiCols );
}

inline bool CGridSizerBuilder::Add( wxWindow* pWindow, const size_t uiRow, const size_t uiCol )
{
	if( !pWindow )
		return false;

	if( uiRow >= m_uiRows )
		return false;

	if( uiCol >= m_uiCols )
		return false;

	m_List[ ( uiRow * m_uiCols ) + uiCol ] = pWindow;

	return true;
}

inline wxFlexGridSizer* CGridSizerBuilder::CreateSizer( const int vgap, const int hgap )
{
	wxFlexGridSizer* pSizer = new wxFlexGridSizer( m_uiRows, m_uiCols, vgap, hgap );

	for( auto& pWindow : m_List )
	{
		if( !pWindow )
		{
			continue;
		}

		pSizer->Add( pWindow );
	}

	return pSizer;
}

#endif //CGRIDSIZERBUILDER_H