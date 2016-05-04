#ifndef HLMV_UI_OPTIONS_COMPILEROPTIONS_H
#define HLMV_UI_OPTIONS_COMPILEROPTIONS_H

#include "../wxHLMV.h"

namespace hlmv
{
class CCompilerOptions final : public wxPanel
{
public:
	CCompilerOptions( wxWindow* pParent, CHLMVSettings* const pSettings );

	void Save();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void Initialize();

	void SetStudioMdl( wxCommandEvent& event );
	void SetMdlDec( wxCommandEvent& event );

private:
	CHLMVSettings* const m_pSettings;

	wxTextCtrl* m_pStudioMdl;
	wxTextCtrl* m_pMdlDec;

private:
	CCompilerOptions( const CCompilerOptions& ) = delete;
	CCompilerOptions& operator=( const CCompilerOptions& ) = delete;
};
}

#endif //HLMV_UI_OPTIONS_COMPILEROPTIONS_H