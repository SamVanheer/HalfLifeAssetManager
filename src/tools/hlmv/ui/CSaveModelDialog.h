#pragma once

#include <wx/dialog.h>

class wxCheckBox;
class wxCommandEvent;
class wxTextCtrl;

namespace hlmv
{
class CSaveModelDialog final : public wxDialog
{
public:
	CSaveModelDialog(wxWindow* parent, const wxString& fileName);

	wxString GetFileName() const;

	void SetFileName(const wxString& fileName);

	bool ShouldCorrectSequenceGroupFileNames() const;

	void SetShouldCorrectSequenceGroupFileNames(bool value);

private:
	void OnBrowseFileName(wxCommandEvent& event);

private:
	wxTextCtrl* m_FileName;
	wxCheckBox* m_CorrectSequenceGroupFileNames;
};
}
