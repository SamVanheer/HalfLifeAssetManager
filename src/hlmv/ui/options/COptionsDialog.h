#ifndef COPTIONSDIALOG_H
#define COPTIONSDIALOG_H

#include <memory>

#include "ui/wxInclude.h"

class wxNotebook;
class CGameConfigurations;

namespace hlmv
{
class CHLMV;
}

class COptionsDialog final : public wxDialog
{
public:
	COptionsDialog( wxWindow* pParent, hlmv::CHLMV* const pHLMV );
	~COptionsDialog();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void OnButton( wxCommandEvent& event );

private:
	hlmv::CHLMV* const m_pHLMV;

	wxNotebook* m_pPages;

	CGameConfigurations* m_pGameConfigs;

private:
	COptionsDialog( const COptionsDialog& ) = delete;
	COptionsDialog& operator=( const COptionsDialog& ) = delete;
};

#endif //COPTIONSDIALOG_H