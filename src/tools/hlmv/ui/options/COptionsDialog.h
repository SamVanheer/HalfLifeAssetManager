#ifndef COPTIONSDIALOG_H
#define COPTIONSDIALOG_H

#include <memory>

#include "../wxHLMV.h"

class wxNotebook;

namespace ui
{
class CGameConfigurationsPanel;
}

namespace hlmv
{
class CHLMVSettings;

class CGeneralOptions;
class CCompilerOptions;

class COptionsDialog final : public wxDialog
{
public:
	COptionsDialog( wxWindow* pParent, CHLMVSettings* const pSettings );
	~COptionsDialog();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void OnButton( wxCommandEvent& event );

private:
	CHLMVSettings* const m_pSettings;
	std::unique_ptr<CHLMVSettings> m_EditableSettings;

	wxNotebook* m_pPages;

	CGeneralOptions* m_pGeneral;
	CCompilerOptions* m_pCompiler;
	ui::CGameConfigurationsPanel* m_pGameConfigs;

private:
	COptionsDialog( const COptionsDialog& ) = delete;
	COptionsDialog& operator=( const COptionsDialog& ) = delete;
};
}

#endif //COPTIONSDIALOG_H