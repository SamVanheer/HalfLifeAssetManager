#ifndef HLMV_CFULLSCREENWINDOW_H
#define HLMV_CFULLSCREENWINDOW_H

#include "wxHLMV.h"

namespace hlmv
{
class CModelViewerApp;
class C3DView;

class CFullscreenWindow final : public wxFrame
{
public:
	CFullscreenWindow( CModelViewerApp* const pHLMV, C3DView* const view3D);
	~CFullscreenWindow();

	void RunFrame();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void KeyDown( wxKeyEvent& event );

private:
	CModelViewerApp* m_pHLMV;

	C3DView* m_p3DView;

private:
	CFullscreenWindow( const CFullscreenWindow& ) = delete;
	CFullscreenWindow& operator=( const CFullscreenWindow& ) = delete;
};
}

#endif //HLMV_CFULLSCREENWINDOW_H