#ifndef COPTIONSDIALOG_H
#define COPTIONSDIALOG_H

#include <memory>

#include "../wxHLMV.h"

#include <wx/propdlg.h>

namespace ui
{
class CGameConfigurationsPanel;
}

namespace hlmv
{
class CHLMVSettings;

class CGeneralOptions;
class CCompilerOptions;

class COptionsDialog final : public wxPropertySheetDialog
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

	CGeneralOptions* m_pGeneral;
	CCompilerOptions* m_pCompiler;
	ui::CGameConfigurationsPanel* m_pGameConfigs;

private:
	COptionsDialog( const COptionsDialog& ) = delete;
	COptionsDialog& operator=( const COptionsDialog& ) = delete;
};
}

#endif //COPTIONSDIALOG_H