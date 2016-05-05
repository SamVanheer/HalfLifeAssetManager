#ifndef HLMV_CFULLSCREENWINDOW_H
#define HLMV_CFULLSCREENWINDOW_H

#include "wxHLMV.h"

class wxNotebook;

namespace hlmv
{
class CHLMV;
class C3DView;

class CFullscreenWindow final : public wxFrame
{
public:
	CFullscreenWindow( CHLMV* const pHLMV, wxNotebook* const pControlPanels );
	~CFullscreenWindow();

	void RunFrame();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void KeyDown( wxKeyEvent& event );

private:
	CHLMV* m_pHLMV;

	C3DView* m_p3DView;

private:
	CFullscreenWindow( const CFullscreenWindow& ) = delete;
	CFullscreenWindow& operator=( const CFullscreenWindow& ) = delete;
};
}

#endif //HLMV_CFULLSCREENWINDOW_H