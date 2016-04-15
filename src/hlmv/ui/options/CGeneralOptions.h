#ifndef HLMV_UI_OPTIONS_CGENERALOPTIONS_H
#define HLMV_UI_OPTIONS_CGENERALOPTIONS_H

#include "hlmv/ui/wxHLMV.h"

class wxColourPickerCtrl;

namespace hlmv
{
class CHLMVSettings;

class CGeneralOptions final : public wxPanel
{
public:
	CGeneralOptions( wxWindow* pParent, CHLMVSettings* const pSettings );
	~CGeneralOptions();

	void Save();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void Initialize();

	void SetDefaultColor( wxCommandEvent& event );

private:
	CHLMVSettings* const m_pSettings;

	wxColourPickerCtrl* m_pGroundColor;
	wxColourPickerCtrl* m_pBackgroundColor;
	wxColourPickerCtrl* m_pCrosshairColor;
	wxColourPickerCtrl* m_pLightColor;
	wxColourPickerCtrl* m_pWireframeColor;

private:
	CGeneralOptions( const CGeneralOptions& ) = delete;
	CGeneralOptions& operator=( const CGeneralOptions& ) = delete;
};
}

#endif //HLMV_UI_OPTIONS_CGENERALOPTIONS_H