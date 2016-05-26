#ifndef HLMV_UI_OPTIONS_COMPILEROPTIONS_H
#define HLMV_UI_OPTIONS_COMPILEROPTIONS_H

#include "../wxHLMV.h"

class wxFilePickerCtrl;
class wxDirPickerCtrl;

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

private:
	CHLMVSettings* const m_pSettings;

	wxFilePickerCtrl* m_pStudioMdl;
	wxFilePickerCtrl* m_pMdlDec;
	wxDirPickerCtrl* m_pDefaultOutputFileDir;

private:
	CCompilerOptions( const CCompilerOptions& ) = delete;
	CCompilerOptions& operator=( const CCompilerOptions& ) = delete;
};
}

#endif //HLMV_UI_OPTIONS_COMPILEROPTIONS_H