#include <algorithm>

#include <wx/gbsizer.h>

#include "wxUtil.h"

namespace wx
{
bool CompareVideoModeResolutions( const wxVideoMode& lhs, const wxVideoMode& rhs )
{
	return lhs.GetWidth() == rhs.GetWidth() && lhs.GetHeight() == lhs.GetHeight();
}

bool GetUniqueVideoModes( const wxDisplay& display, std::vector<wxVideoMode>& videoModes, const VideoModeComparator comparator )
{
	videoModes.clear();

	const wxArrayVideoModes modes = display.GetModes();

	for( size_t uiIndex = 0; uiIndex < modes.Count(); ++uiIndex )
	{
		videoModes.push_back( modes[ uiIndex ] );
	}

	videoModes.erase( std::unique( videoModes.begin(), videoModes.end(), comparator ), videoModes.end() );

	return true;
}

bool GetUniqueVideoModes( const wxDisplay& display, std::vector<wxVideoMode>& videoModes )
{
	return GetUniqueVideoModes( display, videoModes, &CompareVideoModeResolutions );
}

wxColor ColorTowx( const Color& color )
{
	return wxColor( color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
}

Color wxToColor( const wxColor& color )
{
	return Color( color.Red(), color.Green(), color.Blue(), color.Alpha() );
}

bool LaunchDefaultTextEditor( const wxString& szFilename )
{
	const bool bResult = wxLaunchDefaultApplication( szFilename );

	if( !bResult )
	{
		wxMessageBox( "Unable to start default text editor", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
	}

	return bResult;
}

wxSizer* CreateCheckBoxSizer( wxCheckBox** ppCheckBoxes, const size_t uiNumCheckBoxes, const size_t uiNumColumns, int flag, int border )
{
	wxASSERT( ppCheckBoxes );
	wxASSERT( uiNumCheckBoxes > 0 );

	auto pSizer = new wxGridBagSizer(1, 1);

	const int iCheckBoxesPerRow = ceil( static_cast<float>( uiNumCheckBoxes ) / uiNumColumns );

	for( size_t uiIndex = 0; uiIndex < uiNumCheckBoxes; ++uiIndex )
	{
		pSizer->Add( ppCheckBoxes[ uiIndex ], wxGBPosition( uiIndex % iCheckBoxesPerRow, uiIndex / iCheckBoxesPerRow ), wxGBSpan( 1, 1 ), flag, border );
	}

	return pSizer;
}
}