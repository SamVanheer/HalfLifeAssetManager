#ifndef COPTIONSDIALOG_H
#define COPTIONSDIALOG_H

#include <memory>

#include "ui/wxInclude.h"

class wxNotebook;

namespace ui
{
class CGameConfigurations;
}

namespace hlmv
{
class CHLMVSettings;

class CGeneralOptions;
}

class COptionsDialog final : public wxDialog
{
public:
	COptionsDialog( wxWindow* pParent, hlmv::CHLMVSettings* const pSettings );
	~COptionsDialog();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void OnButton( wxCommandEvent& event );

private:
	hlmv::CHLMVSettings* const m_pSettings;
	std::unique_ptr<hlmv::CHLMVSettings> m_EditableSettings;

	wxNotebook* m_pPages;

	hlmv::CGeneralOptions* m_pGeneral;
	ui::CGameConfigurations* m_pGameConfigs;

private:
	COptionsDialog( const COptionsDialog& ) = delete;
	COptionsDialog& operator=( const COptionsDialog& ) = delete;
};

#endif //COPTIONSDIALOG_H