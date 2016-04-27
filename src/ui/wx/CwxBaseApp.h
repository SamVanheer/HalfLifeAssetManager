#ifndef UI_CWXBASEAPP_H
#define UI_CWXBASEAPP_H

#include "wxInclude.h"

namespace tools
{
class CBaseTool;
}

/**
*	Base class for wxWidgets apps using the tools codebase.
*/
class CwxBaseApp : public wxApp
{
protected:
	CwxBaseApp();
	virtual ~CwxBaseApp() = 0;

public:
	virtual bool OnInit() override;

	virtual int OnExit() override;

	tools::CBaseTool* GetTool() { return m_pTool; }

private:
	bool Initialize();

	void Shutdown();

protected:
	/**
	*	Creates the tool. Must return a new instance every time it's called.
	*/
	virtual tools::CBaseTool* CreateTool() = 0;

	/**
	*	Called after OnInit finishes.
	*/
	virtual bool PostInitialize() { return true; }

	/**
	*	Called before OnExit runs shutdown code.
	*/
	virtual void PreShutdown() {}

private:
	tools::CBaseTool* m_pTool = nullptr;

private:
	CwxBaseApp( const CwxBaseApp& ) = delete;
	CwxBaseApp& operator=( const CwxBaseApp& ) = delete;
};

#endif //UI_CWXBASEAPP_H