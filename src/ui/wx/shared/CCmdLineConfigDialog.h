#ifndef UI_WX_SHARED_CCMDLINECONFIGDIALOG_H
#define UI_WX_SHARED_CCMDLINECONFIGDIALOG_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../wxInclude.h"

#include "settings/CCmdLineConfig.h"

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
						  const wxString& szDefaultOutputFileDir,
						  std::shared_ptr<settings::CCmdLineConfigManager> manager,
						  const wxPoint& pos = wxDefaultPosition,
						  const wxSize& size = wxDefaultSize,
						  long style = wxDEFAULT_DIALOG_STYLE,
						  const wxString& name = wxDialogNameStr );

	~CCmdLineConfigDialog();

	/**
	*	@return Whether file copying support is enabled or not.
	*/
	bool IsCopySupportEnabled() const { return m_bCopyOutputFilesEnabled; }

	/**
	*	Sets whether file copying support is enabled or not.
	*/
	void SetCopySupportEnabled( const bool bEnabled );

	/**
	*	Sets the current config by name.
	*/
	void SetConfig( const char* const pszName );

	/**
	*	Sets the current config by index.
	*/
	void SetConfig( int iIndex );

	void Save();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void Initialize();

	void OnConfigChanged( wxCommandEvent& event );

	void OnEditConfig( wxCommandEvent& event );

	void OnAddConfig( wxCommandEvent& event );

	void OnRemoveConfig( wxCommandEvent& event );

private:
	const wxString m_szDefaultOutputFileDir;

	//Original settings that will be updated on OK.
	std::shared_ptr<settings::CCmdLineConfigManager> m_Manager;

	//Settings that can be modified at will.
	std::shared_ptr<settings::CCmdLineConfigManager> m_MutableManager;

	wxChoice* m_pConfigs;

	wxButton* m_pEditConfig;
	wxButton* m_pRemoveConfig;

	bool m_bCopyOutputFilesEnabled = false;

private:
	CCmdLineConfigDialog( const CCmdLineConfigDialog& ) = delete;
	CCmdLineConfigDialog& operator=( const CCmdLineConfigDialog& ) = delete;
};
}

#endif //UI_WX_SHARED_CCMDLINECONFIGDIALOG_H