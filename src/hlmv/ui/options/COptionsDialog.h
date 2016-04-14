#ifndef COPTIONSDIALOG_H
#define COPTIONSDIALOG_H

#include <memory>

#include "ui/wxInclude.h"

class wxNotebook;
class CGameConfigurations;

namespace hlmv
{
class CHLMVState;
}

class COptionsDialog final : public wxDialog
{
public:
	COptionsDialog( wxWindow* pParent, hlmv::CHLMVState* const pSettings );
	~COptionsDialog();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void OnButton( wxCommandEvent& event );

private:
	hlmv::CHLMVState* const m_pSettings;

	wxNotebook* m_pPages;

	CGameConfigurations* m_pGameConfigs;

private:
	COptionsDialog( const COptionsDialog& ) = delete;
	COptionsDialog& operator=( const COptionsDialog& ) = delete;
};

#endif //COPTIONSDIALOG_H