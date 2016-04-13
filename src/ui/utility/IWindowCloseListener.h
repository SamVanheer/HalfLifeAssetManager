#ifndef UI_IWINDOWCLOSELISTENER_H
#define UI_IWINDOWCLOSELISTENER_H

class wxFrame;
class wxCloseEvent;

/**
*	Listener that can be used by window to notify listeners when it is closed. Differs from events in that it can be used to inform listeners if it will actually close or not.
*/
class IWindowCloseListener
{
public:
	virtual ~IWindowCloseListener() = 0;

	/**
	*	Called when the window is closed.
	*	@param pWindow Window that received the close event.
	*	@param event wxCloseEvent that the window received.
	*/
	virtual void OnWindowClose( wxFrame* pWindow, wxCloseEvent& event ) = 0;
};

inline IWindowCloseListener::~IWindowCloseListener()
{
}

#endif //UI_IWINDOWCLOSELISTENER_H