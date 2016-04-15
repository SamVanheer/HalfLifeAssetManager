#ifndef UI_SHARED_CEDITGAMECONFIGSDIALOG_H
#define UI_SHARED_CEDITGAMECONFIGSDIALOG_H

#include <memory>

#include "ui/wxInclude.h"

namespace settings
{
class CGameConfigManager;
}

class wxListView;
class wxListEvent;

namespace ui
{
class CGameConfigurations;

class CEditGameConfigsDialog final : public wxDialog
{
public:
	CEditGameConfigsDialog( wxWindow* pParent, std::shared_ptr<settings::CGameConfigManager> manager, CGameConfigurations* const pGameConfigsPanel );
	~CEditGameConfigsDialog();

	bool AddConfig( const wxString& szName );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void Initialize();

	void OnListColumnBeginDrag( wxListEvent& event );
	void AddConfig( wxCommandEvent& event );
	void EditConfig( wxCommandEvent& event );
	void RemoveConfig( wxCommandEvent& event );

private:
	std::shared_ptr<settings::CGameConfigManager> m_Manager;

	CGameConfigurations* const m_pGameConfigsPanel;

	wxListView* m_pConfigs;

private:
	CEditGameConfigsDialog( const CEditGameConfigsDialog& ) = delete;
	CEditGameConfigsDialog& operator=( const CEditGameConfigsDialog& ) = delete;
};
}

#endif //UI_SHARED_CEDITGAMECONFIGSDIALOG_H