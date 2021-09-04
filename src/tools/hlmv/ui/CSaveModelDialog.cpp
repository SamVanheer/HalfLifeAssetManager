#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/gbsizer.h>
#include <wx/platinfo.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "ui/CSaveModelDialog.h"

namespace hlmv
{
CSaveModelDialog::CSaveModelDialog(wxWindow* parent, const wxString& fileName)
	: wxDialog(parent, wxID_ANY, "Save Model")
{
	m_FileName = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(400, wxDefaultSize.GetHeight()));

	auto browseFileNameButton = new wxButton(this, wxID_ANY, "...");

	browseFileNameButton->Bind(wxEVT_BUTTON, &CSaveModelDialog::OnBrowseFileName, this);

	m_CorrectSequenceGroupFileNames = new wxCheckBox(this, wxID_ANY, "Correct Sequence Group Filenames");

	//Disable this feature on Windows XP (will not work)
	if ((wxPlatformInfo::Get().GetOperatingSystemId() & wxOperatingSystemId::wxOS_WINDOWS) && !wxPlatformInfo::Get().CheckOSVersion(6, 0))
	{
		m_CorrectSequenceGroupFileNames->Disable();
	}

	auto buttonsPanel = new wxPanel(this);

	auto saveButton = new wxButton(buttonsPanel, wxID_OK, "Save");

	auto cancelButton = new wxButton(buttonsPanel, wxID_CANCEL, "Cancel");

	auto sizer = new wxGridBagSizer(1, 1);

	sizer->Add(new wxStaticText(this, wxID_ANY, "Filename:"), wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer->Add(m_FileName, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer->Add(browseFileNameButton, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND);

	sizer->Add(m_CorrectSequenceGroupFileNames, wxGBPosition(1, 0), wxGBSpan(1, 3), wxEXPAND);

	{
		auto panelSizer = new wxBoxSizer(wxHORIZONTAL);

		panelSizer->AddStretchSpacer();

		panelSizer->Add(saveButton, wxSizerFlags().Expand());
		panelSizer->Add(cancelButton, wxSizerFlags().Expand());

		buttonsPanel->SetSizer(panelSizer);

		sizer->Add(buttonsPanel, wxGBPosition(2, 0), wxGBSpan(1, 3), wxEXPAND);
	}

	SetSizerAndFit(sizer);

	CenterOnScreen();

	SetFileName(fileName);
}

wxString CSaveModelDialog::GetFileName() const
{
	return m_FileName->GetValue();
}

void CSaveModelDialog::SetFileName(const wxString& fileName)
{
	auto absoluteFileName{wxFileName::FileName(fileName)};

	absoluteFileName.MakeAbsolute();

	m_FileName->SetValue(absoluteFileName.GetFullPath());
}

bool CSaveModelDialog::ShouldCorrectSequenceGroupFileNames() const
{
	return m_CorrectSequenceGroupFileNames->GetValue();
}

void CSaveModelDialog::SetShouldCorrectSequenceGroupFileNames(bool value)
{
	if (m_CorrectSequenceGroupFileNames->IsEnabled())
	{
		m_CorrectSequenceGroupFileNames->SetValue(value);
	}
}

void CSaveModelDialog::OnBrowseFileName(wxCommandEvent& event)
{
	auto fileName{wxFileName::FileName(GetFileName())};

	fileName.MakeAbsolute();

	wxFileDialog dlg(this, wxFileSelectorPromptStr, fileName.GetPath(), fileName.GetName(), "Half-Life Models (*.mdl)|*.mdl", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (dlg.ShowModal() != wxID_CANCEL)
	{
		SetFileName(dlg.GetPath());
	}
}
}
