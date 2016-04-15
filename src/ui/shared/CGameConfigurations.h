#ifndef UI_SHARED_CGAMECONFIGURATIONS_H
#define UI_SHARED_CGAMECONFIGURATIONS_H

#include <memory>

#include "ui/wxInclude.h"

namespace settings
{
class CGameConfigManager;
}

namespace ui
{
//TODO: rename to CGameConfigurationsPanel
class CGameConfigurations final : public wxPanel
{
public:
	CGameConfigurations( wxWindow* pParent, std::shared_ptr<settings::CGameConfigManager> manager );
	~CGameConfigurations();

	void Save();

	//Used by CEditGameConfigsDialog to inform us of changes.
	void ConfigAdded( const wxString& szConfigName );

	void ConfigRenamed( const wxString& szOldName, const wxString& szNewName );

	void ConfigRemoved( const wxString& szConfigName );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void Initialize();

	void SetCurrentConfig( int iIndex );

	void StoreConfig( const unsigned int uiIndex );

	void ConfigChanged( wxCommandEvent& event );

	void EditConfigs( wxCommandEvent& event );

	void FindBasePath( wxCommandEvent& event );

private:
	std::shared_ptr<settings::CGameConfigManager> m_Manager;

	wxComboBox* m_pActiveConfig;

	wxComboBox* m_pConfigs;

	//wxComboBox does not track the previous selection.
	int m_iCurrentConfig = wxNOT_FOUND;

	wxTextCtrl* m_pBasePath;
	wxButton* m_pFindBasePath;
	wxTextCtrl* m_pGameDir;
	wxTextCtrl* m_pModDir;

private:
	CGameConfigurations( const CGameConfigurations& ) = delete;
	CGameConfigurations& operator=( const CGameConfigurations& ) = delete;
};
}

#endif //UI_SHARED_CGAMECONFIGURATIONS_H