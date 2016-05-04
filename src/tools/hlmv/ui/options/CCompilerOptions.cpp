#include <wx/gbsizer.h>
#include <wx/clrpicker.h>
#include <wx/statline.h>

#include "../../settings/CHLMVSettings.h"

#include "CCompilerOptions.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CCompilerOptions, wxPanel )
	EVT_BUTTON( wxID_OPTIONS_COMPILER_STUDIOMDL, CCompilerOptions::SetStudioMdl )
	EVT_BUTTON( wxID_OPTIONS_COMPILER_MDLDEC, CCompilerOptions::SetMdlDec )
wxEND_EVENT_TABLE()

CCompilerOptions::CCompilerOptions( wxWindow* pParent, CHLMVSettings* const pSettings )
	: wxPanel( pParent )
	, m_pSettings( pSettings )
{
	wxASSERT( pSettings );

	auto pStudioMdlText = new wxStaticText( this, wxID_ANY, "Path to \'studiomdl.exe\'" );
	m_pStudioMdl = new wxTextCtrl( this, wxID_ANY );
	auto pStudioMdlButton = new wxButton( this, wxID_OPTIONS_COMPILER_STUDIOMDL, "...", wxDefaultPosition, wxSize( 25, wxDefaultSize.GetHeight() ) );

	auto pMdlDecText = new wxStaticText( this, wxID_ANY, "Path to \'mdldec.exe\'" );
	m_pMdlDec = new wxTextCtrl( this, wxID_ANY );
	auto pSMdlDecButton = new wxButton( this, wxID_OPTIONS_COMPILER_MDLDEC, "...", wxDefaultPosition, wxSize( 25, wxDefaultSize.GetHeight() ) );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	int iRow = 0;

	const int iNumCols = 10;

	const int iButtonCols = 1;

	pSizer->Add( pStudioMdlText, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, iNumCols ), wxEXPAND );
	pSizer->Add( m_pStudioMdl, wxGBPosition( iRow, 0 ), wxGBSpan( 1, iNumCols - iButtonCols ), wxEXPAND );
	pSizer->Add( pStudioMdlButton, wxGBPosition( iRow++, iNumCols - iButtonCols ), wxGBSpan( 1, iButtonCols ), wxEXPAND );

	pSizer->Add( pMdlDecText, wxGBPosition( iRow++, 0 ), wxGBSpan( 1, iNumCols ), wxEXPAND );
	pSizer->Add( m_pMdlDec, wxGBPosition( iRow, 0 ), wxGBSpan( 1, iNumCols - iButtonCols ), wxEXPAND );
	pSizer->Add( pSMdlDecButton, wxGBPosition( iRow++, iNumCols - iButtonCols ), wxGBSpan( 1, iButtonCols ), wxEXPAND );

	for( size_t uiIndex = 0; uiIndex < static_cast<size_t>( iNumCols ); ++uiIndex )
	{
		pSizer->AddGrowableCol( uiIndex );
	}

	this->SetSizer( pSizer );

	Initialize();
}

void CCompilerOptions::Save()
{
	m_pSettings->SetStudioMdl( m_pStudioMdl->GetValue().c_str().AsChar() );
	m_pSettings->SetMdlDec( m_pMdlDec->GetValue().c_str().AsChar() );
}

void CCompilerOptions::Initialize()
{
	m_pStudioMdl->ChangeValue( m_pSettings->GetStudioMdl().CStr() );
	m_pMdlDec->ChangeValue( m_pSettings->GetMdlDec().CStr() );
}

void CCompilerOptions::SetStudioMdl( wxCommandEvent& event )
{
	wxFileDialog dlg( this, "Select studiomdl", wxEmptyString, wxEmptyString, 
						"Executables (*.exe;*.com)|*.exe;*.com"
						"All files (*.*)|*.*" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pStudioMdl->ChangeValue( dlg.GetPath() );
}

void CCompilerOptions::SetMdlDec( wxCommandEvent& event )
{
	wxFileDialog dlg( this, "Select mdldec", wxEmptyString, wxEmptyString,
					  "Executables (*.exe;*.com)|*.exe;*.com"
					  "All files (*.*)|*.*" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	m_pMdlDec->ChangeValue( dlg.GetPath() );
}
}