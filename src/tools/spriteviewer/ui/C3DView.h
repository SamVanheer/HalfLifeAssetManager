#ifndef SPRITEVIEWER_UI_C3DVIEW_H
#define SPRITEVIEWER_UI_C3DVIEW_H

#include "wxSpriteViewer.h"

#include "ui/wx/shared/CwxBaseGLCanvas.h"

#include <glm/vec3.hpp>

#include "graphics/Constants.h"

namespace sprview
{
class CSpriteViewerApp;

class I3DViewListener
{
public:
	virtual ~I3DViewListener() = 0;

	virtual void Draw3D( const wxSize& size ) = 0;
};

inline I3DViewListener::~I3DViewListener()
{
}

class C3DView final : public ui::CwxBaseGLCanvas
{
public:
	C3DView( wxWindow* pParent, CSpriteViewerApp* const pSpriteViewer, I3DViewListener* pListener = nullptr );
	~C3DView();

	const CSpriteViewerApp* GetSpriteViewer() const { return m_pSpriteViewer; }
	CSpriteViewerApp* GetSpriteViewer() { return m_pSpriteViewer; }

	//Tells the 3D view to prepare for model loading.
	void PrepareForLoad();

	void UpdateView();

	bool LoadBackgroundTexture( const wxString& szFilename );
	void UnloadBackgroundTexture();

	void TakeScreenshot();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	void DrawScene() override final;

	void DrawSpriteInfo();

	void DrawSprite();

private:
	CSpriteViewerApp* const m_pSpriteViewer;

	I3DViewListener* m_pListener;

	GLuint m_BackgroundTexture	= GL_INVALID_TEXTURE_ID;

private:
	C3DView( const C3DView& ) = delete;
	C3DView& operator=( const C3DView& ) = delete;
};
}

#endif //SPRITEVIEWER_UI_C3DVIEW_H