#include "CMainWindow.h"

#include <wx/display.h>
#include <wx/image.h>

#include <algorithm>
#include <vector>

#include "CModelViewerApp.h"

wxIMPLEMENT_APP( CModelViewerApp );

CModelViewerApp* CModelViewerApp::m_pInstance = nullptr;

bool CModelViewerApp::OnInit()
{
	m_pInstance = this;

	wxInitAllImageHandlers();

	m_pPrimaryDisplay = new wxDisplay( 0 );

	wxArrayVideoModes modes = m_pPrimaryDisplay->GetModes();

	for( size_t uiIndex = 0; uiIndex < modes.Count(); ++uiIndex )
	{
		m_VideoModes.push_back( modes[ uiIndex ] );
	}

	m_VideoModes.erase( std::unique( m_VideoModes.begin(), m_VideoModes.end(), []( const wxVideoMode& lhs, const wxVideoMode& rhs )
	{
		return lhs.GetWidth() == rhs.GetWidth() && lhs.GetHeight() == lhs.GetHeight();
	}
	), m_VideoModes.end() );

	CMainWindow* pFrame = new CMainWindow();
	pFrame->Show( true );

	return true;
}

int CModelViewerApp::OnExit()
{
	delete m_pPrimaryDisplay;

	m_pInstance = nullptr;

	return wxApp::OnExit();
}