#include "CwxBaseFrame.h"

namespace ui
{
CwxBaseFrame::CwxBaseFrame( wxWindow* pParent, wxWindowID id, const wxString& szTitle,
				   const wxPoint& pos, const wxSize& size,
				   long style, const wxString& szName )
	: wxFrame( pParent, id, szTitle, pos, size, style, szName )
	, m_szTitleBase( szTitle )
{
}

CwxBaseFrame::~CwxBaseFrame()
{
}

void CwxBaseFrame::SetTitleBase( const wxString& szTitleBase )
{
	m_szTitleBase = szTitleBase;

	RefreshTitleContent();
}

void CwxBaseFrame::SetTitleContent( const wxString& szTitleContent )
{
	m_szTitleContent = szTitleContent;

	RefreshTitleContent();
}

void CwxBaseFrame::RefreshTitleContent()
{
	wxString szTitle = m_szTitleBase;

	if( !m_szTitleContent.IsEmpty() )
	{
		szTitle += " - ";
		szTitle += m_szTitleContent;
	}

	SetTitle( szTitle );
}
}