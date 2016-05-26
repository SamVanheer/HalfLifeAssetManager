#ifndef UI_SHARED_CEDITCMDLINECONFIGDIALOG_H
#define UI_SHARED_CEDITCMDLINECONFIGDIALOG_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../wxInclude.h"

#include <wx/filepicker.h>

#include "settings/CCmdLineConfig.h"

#include <wx/propgrid/propgrid.h>

namespace ui
{
/**
*	@brief A dialog that can be used to edit a command line configuration.
*/
class CEditCmdLineConfigDialog : public wxDialog
{
public:
	CEditCmdLineConfigDialog( wxWindow *parent, wxWindowID id,
							  std::shared_ptr<settings::CCmdLineConfigManager> mutableManager,
						  std::shared_ptr<settings::CCmdLineConfig> config,
						  const wxPoint& pos = wxDefaultPosition,
						  const wxSize& size = wxDefaultSize,
						  long style = wxDEFAULT_DIALOG_STYLE,
						  const wxString& name = wxDialogNameStr );

	~CEditCmdLineConfigDialog();

	/**
	*	Sets whether file copying support is enabled or not.
	*/
	void SetCopySupportEnabled( const bool bEnabled );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void Initialize();

	bool Save();

	void InitFromConfig( const std::shared_ptr<const settings::CCmdLineConfig>& config );

	void SaveToConfig( const std::shared_ptr<settings::CCmdLineConfig>& config );

	/**
	*	@return A vector of pairs containing the command line parameters.
	*/
	std::vector<std::pair<std::string, std::string>> GetParameters() const;

	/**
	*	@return Whether or not to copy output files on completion.
	*/
	bool ShouldCopyFiles() const;

	/**
	*	@return List of filters to use for output file copying.
	*/
	wxArrayString GetOutputFileFilters() const;

	void UpdateTitle();

	void OnAddParameter( wxCommandEvent& event );

	void OnRemoveParameter( wxCommandEvent& event );

	void OnCopyFilesChanged( wxCommandEvent& event );

	void OnButtonPressed( wxCommandEvent& event );

private:
	//Manager that contains the config. Will be updated on OK.
	std::shared_ptr<settings::CCmdLineConfigManager> m_MutableManager;

	//Original config that will be updated on OK.
	std::shared_ptr<settings::CCmdLineConfig> m_Config;

	//Config that can be modified at will.
	std::shared_ptr<settings::CCmdLineConfig> m_MutableConfig;

	wxTextCtrl* m_pName;

	wxPropertyGrid* m_pParameterGrid;

	wxButton* m_pRemoveParam;

	wxCheckBox* m_pCopyFiles;

	wxDirPickerCtrl* m_pOutputFileDir;

	wxEditableListBox* m_pOutputFilters;

private:
	CEditCmdLineConfigDialog( const CEditCmdLineConfigDialog& ) = delete;
	CEditCmdLineConfigDialog& operator=( const CEditCmdLineConfigDialog& ) = delete;
};
}

#endif //UI_WX_SHARED_CEDITCMDLINECONFIGDIALOG_H