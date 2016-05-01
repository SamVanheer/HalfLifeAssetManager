#ifndef UI_WX_SHARED_CWXBASEFRAME_H
#define UI_WX_SHARED_CWXBASEFRAME_H

#include "ui/wx/wxInclude.h"

namespace ui
{
/**
*	Base class for frames. Can have a title comprised out of a base and content.
*	Abstract.
*	@see CwxBaseFrame::SetTitleBase
*/
class CwxBaseFrame : public wxFrame
{
protected:
	/**
	*	Constructor.
	*	@param pParent Parent window.
	*	@param id Window ID.
	*	@param szTitle Window title. Also sets the title base.
	*	@param pos Window position.
	*	@param size Window size.
	*	@param style Window styles.
	*	@param szName Window name.
	*	@see SetTitleBase
	*/
	CwxBaseFrame( wxWindow* pParent, wxWindowID id, const wxString& szTitle,
					   const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, 
					   long style = wxDEFAULT_FRAME_STYLE, const wxString& szName = wxFrameNameStr );

public:
	/**
	*	Destructor.
	*/
	virtual ~CwxBaseFrame() = 0;

	/**
	*	Gets the title base.
	*	@return Title base.
	*/
	const wxString& GetTitleBase() const { return m_szTitleBase; }

	/**
	*	Sets the title base. The title base and content are combined to form a complete title.
	*	<pre>
	*	For example, when the base is set to "Model Viewer" and the content is set to "chair.mdl", the title is set to "Model Viewer - chair.mdl"
	*	</pre>
	*	@param szTitleBase Base to set.
	*/
	void SetTitleBase( const wxString& szTitleBase );

	/**
	*	Gets the title content.
	*	@return Title content.
	*/
	const wxString& GetTitleContent() const { return m_szTitleContent; }

	/**
	*	Sets the title content.
	*	@param szTitleContent Content to set.
	*	@see SetTitleBase
	*/
	void SetTitleContent( const wxString& szTitleContent );

	/**
	*	Clears the title content. Convenience method for SetTitleContent( "" ).
	*/
	void ClearTitleContent() { SetTitleContent( "" ); }

	/**
	*	Refreshes the title to contain the title content string. Should be used only if the title was changed externally.
	*	@see SetTitleBase
	*/
	void RefreshTitleContent();

private:
	/**
	*	Base part of the title. When the title is changed, it is set to the base + " - <content>"
	*/
	wxString m_szTitleBase;

	/**
	*	Content part of the title.
	*/
	wxString m_szTitleContent;

private:
	CwxBaseFrame( const CwxBaseFrame& ) = delete;
	CwxBaseFrame& operator=( const CwxBaseFrame& ) = delete;
};
}

#endif //UI_WX_SHARED_CWXBASEFRAME_H