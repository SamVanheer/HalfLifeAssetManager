#ifndef UI_WX_SHARED_CCMDLINECONFIGDIALOG_H
#define UI_WX_SHARED_CCMDLINECONFIGDIALOG_H

#include <string>
#include <utility>
#include <vector>

#include "../wxInclude.h"

#include <wx/propgrid/propgrid.h>

namespace ui
{
/**
*	@brief A dialog that can be used to configure a command line program.
*/
class CCmdLineConfigDialog : public wxDialog
{
public:
	CCmdLineConfigDialog( wxWindow *parent, wxWindowID id,
						  const wxString& title,
						  const wxPoint& pos = wxDefaultPosition,
						  const wxSize& size = wxDefaultSize,
						  long style = wxDEFAULT_DIALOG_STYLE,
						  const wxString& name = wxDialogNameStr );

	~CCmdLineConfigDialog();

	/**
	*	@return A vector of pairs containing the command line parameters.
	*/
	std::vector<std::pair<std::string, std::string>> GetParameters() const;

	/**
	*	@return Whether or not to copy output files on completion.
	*/
	bool ShouldCopyFiles() const;

	/**
	*	Sets whether file copying support is enabled or not.
	*/
	void SetCopySupportEnabled( const bool bEnabled );

	/**
	*	@return List of filters to use for output file copying.
	*/
	wxArrayString GetOutputFileFilters() const;

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void OnAddParameter( wxCommandEvent& event );

	void OnEditParameterName( wxCommandEvent& event );

	void OnRemoveParameter( wxCommandEvent& event );

	void OnCopyFilesChanged( wxCommandEvent& event );

private:
	wxPropertyGrid* m_pParameterGrid;

	wxCheckBox* m_pCopyFiles;

	wxEditableListBox* m_pOutputFilters;

private:
	CCmdLineConfigDialog( const CCmdLineConfigDialog& ) = delete;
	CCmdLineConfigDialog& operator=( const CCmdLineConfigDialog& ) = delete;
};
}

#endif //UI_WX_SHARED_CCMDLINECONFIGDIALOG_H