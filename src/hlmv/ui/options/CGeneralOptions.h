#ifndef HLMV_UI_OPTIONS_CGENERALOPTIONS_H
#define HLMV_UI_OPTIONS_CGENERALOPTIONS_H

#include "hlmv/ui/wxHLMV.h"

namespace hlmv
{
class CHLMVSettings;

class CGeneralOptions final : public wxPanel
{
public:
	CGeneralOptions( wxWindow* pParent, CHLMVSettings* const pSettings );
	~CGeneralOptions();

	void Save();

private:
	CHLMVSettings* const m_pSettings;

private:
	CGeneralOptions( const CGeneralOptions& ) = delete;
	CGeneralOptions& operator=( const CGeneralOptions& ) = delete;
};
}

#endif //HLMV_UI_OPTIONS_CGENERALOPTIONS_H