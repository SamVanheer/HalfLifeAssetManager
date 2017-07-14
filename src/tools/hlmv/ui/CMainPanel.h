#ifndef CMAINPANEL_H
#define CMAINPANEL_H

#include "wxHLMV.h"

#include <wx/notebook.h>

#include "shared/Utility.h"

#include "shared/renderer/studiomodel/IStudioModelRendererListener.h"

#include "controlpanels/CBaseControlPanel.h"

#include "C3DView.h"

namespace hlmv
{
class CModelViewerApp;

class CModelDisplayPanel;
class CGlobalFlagsPanel;
class CBodyPartsPanel;
class CBonesPanel;
class CAttachmentsPanel;
class CTexturesPanel;
class CSequencesPanel;
class CFullscreenPanel;

class CMainPanel final : public wxPanel, public I3DViewListener, public studiomdl::IStudioModelRendererListener
{
public:

	//Weakly typed, strongly scoped.
	class CheckBox
	{
	public:
		enum Type
		{
			FIRST				= 0,
			SHOW_HITBOXES		= FIRST,
			SHOW_GROUND,
			SHOW_BONES,
			MIRROR_ON_GROUND,
			SHOW_ATTACHMENTS,
			SHOW_BACKGROUND,
			SHOW_EYE_POSITION,
			WIREFRAME_OVERLAY,

			COUNT,
			LAST				= COUNT - 1	//Must be last.
		};

	private:
		CheckBox() = delete;
		CheckBox( const CheckBox& ) = delete;
		CheckBox& operator=( const CheckBox& ) = delete;
	};

	static const size_t CONTROLS_ROWS = 4;
	static const size_t CONTROLS_COLS = 5;

	static const size_t NUM_CHECKBOXES_PER_ROW = 2;

	static const size_t OPACITY_MIN = 0;
	static const size_t OPACITY_MAX = 100;
	static const size_t OPACITY_DEFAULT = OPACITY_MAX;

	static const glm::vec3 DEFAULT_LIGHT_VECTOR;

public:
	CMainPanel( wxWindow* pParent, CModelViewerApp* const pHLMV );
	~CMainPanel();

	const CModelViewerApp* GetHLMV() const { return m_pHLMV; }
	CModelViewerApp* GetHLMV() { return m_pHLMV; }

	void RunFrame();

	void Draw3D( const wxSize& size ) override final;

	wxNotebook* GetControlPanels() { return m_pControlPanels; }

	CModelDisplayPanel*		GetModelDisplayPanel() { return m_pModelDisplay; }
	CGlobalFlagsPanel*		GetGlobalFlagsPanel() { return m_pGlobalFlags; }
	CBodyPartsPanel*		GetBodyPartsPanel() { return m_pBodyParts; }
	CBonesPanel*			GetBonesPanel() { return m_pBones; }
	CAttachmentsPanel*		GetAttachmentsPanel() { return m_pAttachments; }
	CTexturesPanel*			GetTexturesPanel() { return m_pTextures; }
	CSequencesPanel*		GetSequencesPanel() { return m_pSequencesPanel; }
	CFullscreenPanel*		GetFullscreenPanel() { return m_pFullscreen; }

	bool LoadModel( const wxString& szFilename );

	void FreeModel();

	void InitializeUI();

	void PageChanged( wxBookCtrlEvent& event );

	bool LoadBackgroundTexture( const wxString& szFilename );
	void UnloadBackgroundTexture();

	bool LoadGroundTexture( const wxString& szFilename );
	void UnloadGroundTexture();

	void SaveUVMap( const wxString& szFilename, const int iTexture );

	void TakeScreenshot();

protected:
	wxDECLARE_EVENT_TABLE();

private:
	/*
	*	Helper function to inform all control panels.
	*	Important: pass by reference.
	*/
	template<typename FUNC, typename... ARGS>
	void ForEachPanel( FUNC func, ARGS&... args )
	{
		const size_t uiPageCount = m_pControlPanels->GetPageCount();

		for( size_t uiIndex = 0; uiIndex < uiPageCount; ++uiIndex )
		{
			CBaseControlPanel* const pPanel = static_cast<CBaseControlPanel*>( m_pControlPanels->GetPage( uiIndex ) );

			( pPanel->*func )( args... );
		}
	}

	void OnPostDraw( studiomdl::IStudioModelRenderer& renderer, const studiomdl::CModelRenderInfo& info ) override final;

	void ViewOriginChanged( wxCommandEvent& event );

	void ResetLightVector( wxCommandEvent& event );

private:
	CModelViewerApp* const m_pHLMV;

	C3DView* m_p3DView;

	wxPanel* m_pControlPanel;

	wxPanel* m_pMainControlBar;

	wxRadioBox* m_pViewOrigin;

	unsigned int m_uiOldDrawnPolys = -1;

	wxStaticText* m_pDrawnPolys;

	long long m_iLastFPSUpdate = GetCurrentTick();
	unsigned int m_uiCurrentFPS = 0;

	wxStaticText* m_pFPS;

	wxStaticText* m_pLightVector;

	glm::vec3 m_vecPrevLightVec;

	wxButton* m_pResetLightVector;

	wxNotebook* m_pControlPanels;

	CModelDisplayPanel*		m_pModelDisplay;
	CGlobalFlagsPanel*		m_pGlobalFlags;
	CBodyPartsPanel*		m_pBodyParts;
	CBonesPanel*			m_pBones;
	CAttachmentsPanel*		m_pAttachments;
	CTexturesPanel*			m_pTextures;
	CSequencesPanel*		m_pSequencesPanel;
	CFullscreenPanel*		m_pFullscreen;

private:
	CMainPanel( const CMainPanel& ) = delete;
	CMainPanel& operator=( const CMainPanel& ) = delete;
};
}

#endif //CMAINPANEL_H