#ifndef TOOLS_SHARED_CBASEWXTOOLAPP_H
#define TOOLS_SHARED_CBASEWXTOOLAPP_H

#include "CBaseToolApp.h"

namespace tools
{
class CBaseWXToolApp : public CBaseToolApp
{
public:

protected:
	bool Connect( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	virtual bool RunApp( int iArgc, wchar_t* pszArgV[] ) override;

	bool InitOpenGL() override;

	/**
	*	Allows the tool to configure custom canvas attributes.
	*/
	virtual void GetGLCanvasAttributes( wxGLAttributes& attrs );

	/**
	*	Allow the tool to configure custom context attributes.
	*/
	virtual void GetGLContextAttributes( wxGLContextAttrs& attrs );

	/**
	*	Allows wxWidgets apps to run code after base tool app run code has been executed, but before the wxWidgets app is started.
	*	@param iArgc Argument count.
	*	@param pszArgV Argument Vector.
	*	@return true on success, false otherwise.
	*/
	virtual bool PreRunApp( int iArgc, wchar_t* pszArgV[] ) { return true; }

	/**
	*	Runs the wxWidgets app. Allows apps to override RunApp entirely and implement the logic themselves.
	*	@see RunApp
	*/
	bool RunWXApp( int iArgc, wchar_t* pszArgV[] );
};
}

#endif //TOOLS_SHARED_CBASEWXTOOLAPP_H