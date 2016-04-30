#ifndef SPRITEVIEWER_UI_CSPRITEVIEWERAPP_H
#define SPRITEVIEWER_UI_CSPRITEVIEWERAPP_H

#include "wxSpriteViewer.h"

#include "ui/wx/CwxBaseApp.h"

#include "CSpriteViewer.h"

class CSpriteViewerApp final : public CwxBaseApp
{
public:
	virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;

	virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

	sprview::CSpriteViewer* GetTool() { return static_cast<sprview::CSpriteViewer*>( CwxBaseApp::GetTool() ); }

private:
	virtual sprview::CSpriteViewer* CreateTool() override { return new sprview::CSpriteViewer(); }

	virtual bool PostInitialize() override;

private:
	wxString m_szSprite;		//Sprite to load on startup, if any.
};

wxDECLARE_APP( CSpriteViewerApp );

#endif //SPRITEVIEWER_UI_CSPRITEVIEWERAPP_H