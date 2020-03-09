#include <wx/gbsizer.h>
#include <wx/clrpicker.h>
#include <wx/statline.h>
#include <wx/filepicker.h>

#include "../../settings/CHLMVSettings.h"

#include "CCompilerOptions.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CCompilerOptions, wxPanel )
wxEND_EVENT_TABLE()

CCompilerOptions::CCompilerOptions( wxWindow* pParent, CHLMVSettings* const pSettings )
	: wxPanel( pParent )
	, m_pSettings( pSettings )
{
	wxASSERT( pSettings );

	auto pStudioMdlText = new wxStaticText( this, wxID_ANY, "Path to \'studiomdl.exe\'" );

	m_pStudioMdl = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxFileSelectorPromptStr, 
										 "Executables (*.exe;*.com)|*.exe;*.com"
										 "|All files (*.*)|*.*", 
										 wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE | wxFLP_USE_TEXTCTRL | wxFLP_SMALL );

	auto pMdlDecText = new wxStaticText( this, wxID_ANY, "Path to \'mdldec.exe\'" );

	m_pMdlDec = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxFileSelectorPromptStr,
									  "Executables (*.exe;*.com)|*.exe;*.com"
									  "|All files (*.*)|*.*", 
									  wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE | wxFLP_USE_TEXTCTRL | wxFLP_SMALL );

	auto pMDLOutputDirText = new wxStaticText( this, wxID_ANY, "Default destination directory for command line output files" );

	m_pDefaultOutputFileDir = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString, wxDirSelectorPromptStr,
										   wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE | wxDIRP_USE_TEXTCTRL | wxDIRP_SMALL );

	//Layout
	auto pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->AddSpacer( 20 );

	pSizer->Add( pStudioMdlText, wxSizerFlags().Expand() );
	pSizer->Add( m_pStudioMdl, wxSizerFlags().Expand() );

	pSizer->AddSpacer( 20 );

	pSizer->Add( pMdlDecText, wxSizerFlags().Expand() );
	pSizer->Add( m_pMdlDec, wxSizerFlags().Expand() );

	pSizer->AddSpacer( 20 );

	pSizer->Add( pMDLOutputDirText, wxSizerFlags().Expand() );
	pSizer->Add( m_pDefaultOutputFileDir, wxSizerFlags().Expand() );

	this->SetSizer( pSizer );

	Initialize();
}

void CCompilerOptions::Save()
{
	m_pSettings->SetStudioMdl( m_pStudioMdl->GetPath().c_str().AsChar() );
	m_pSettings->SetMdlDec( m_pMdlDec->GetPath().c_str().AsChar() );
	m_pSettings->SetDefaultOutputFileDirectory( m_pDefaultOutputFileDir->GetPath().c_str().AsChar() );
}

void CCompilerOptions::Initialize()
{
	m_pStudioMdl->SetPath( m_pSettings->GetStudioMdl() );
	m_pMdlDec->SetPath( m_pSettings->GetMdlDec() );
	m_pDefaultOutputFileDir->SetPath( m_pSettings->GetDefaultOutputFileDirectory() );
}
}