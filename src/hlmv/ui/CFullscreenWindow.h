#ifndef HLMV_CFULLSCREENWINDOW_H
#define HLMV_CFULLSCREENWINDOW_H

#include "wxHLMV.h"

#include "ui/utility/CTimer.h"

class CHLMVSettings;
class C3DView;

namespace hlmv
{
class CFullscreenWindow final : public wxFrame, public ITimerListener
{
public:
	CFullscreenWindow( CHLMVSettings* const pSettings );
	~CFullscreenWindow();

	void OnTimer( CTimer& timer ) override final;

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void KeyDown( wxKeyEvent& event );

private:
	CHLMVSettings* m_pSettings;

	C3DView* m_p3DView;

private:
	CFullscreenWindow( const CFullscreenWindow& ) = delete;
	CFullscreenWindow& operator=( const CFullscreenWindow& ) = delete;
};
}

#endif //HLMV_CFULLSCREENWINDOW_H