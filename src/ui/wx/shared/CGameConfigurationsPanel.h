#ifndef UI_SHARED_CGAMECONFIGURATIONSPANEL_H
#define UI_SHARED_CGAMECONFIGURATIONSPANEL_H

#include <memory>

#include "ui/wx/wxInclude.h"

#include "settings/CBaseSettings.h"

class wxDirPickerCtrl;

namespace ui
{
/**
*	This panel contains the game configurations and active configuration settings.
*/
class CGameConfigurationsPanel final : public wxPanel
{
public:
	CGameConfigurationsPanel( wxWindow* pParent, std::shared_ptr<settings::CGameConfigManager> manager );
	~CGameConfigurationsPanel();

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

private:
	std::shared_ptr<settings::CGameConfigManager> m_Manager;

	wxChoice* m_pActiveConfig;

	wxChoice* m_pConfigs;

	//wxChoice does not track the previous selection.
	int m_iCurrentConfig = wxNOT_FOUND;

	wxDirPickerCtrl* m_pBasePath;
	wxTextCtrl* m_pGameDir;
	wxTextCtrl* m_pModDir;

private:
	CGameConfigurationsPanel( const CGameConfigurationsPanel& ) = delete;
	CGameConfigurationsPanel& operator=( const CGameConfigurationsPanel& ) = delete;
};
}

#endif //UI_SHARED_CGAMECONFIGURATIONSPANEL_H