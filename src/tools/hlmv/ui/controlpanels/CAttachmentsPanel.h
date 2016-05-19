#ifndef HLMV_UI_CONTROLPANELS_CATTACHMENTSPANEL_H
#define HLMV_UI_CONTROLPANELS_CATTACHMENTSPANEL_H

#include "engine/shared/studiomodel/studio.h"

#include "CBaseControlPanel.h"

namespace hlmv
{
/**
*	@brief Attachments control panel for studio models.
*/
class CAttachmentsPanel final : public CBaseControlPanel
{
public:
	/**
	*	Constructor.
	*	@param pParent Parent window.
	*	@param pHLMV HLMV instance.
	*/
	CAttachmentsPanel( wxWindow* pParent, CHLMV* const pHLMV );

	/**
	*	Destructor.
	*/
	~CAttachmentsPanel();

	void InitializeUI() override final;

	void OnPostDraw( studiomdl::IStudioModelRenderer& renderer, const studiomdl::CModelRenderInfo& info ) override final;

	/**
	*	Sets the attachment whose info should be displayed.
	*	@param iIndex Attachment index. Must be valid.
	*/
	void SetAttachment( int iIndex );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	/**
	*	Handler for attachment change events.
	*	@param event Event.
	*/
	void OnAttachmentChanged( wxCommandEvent& event );

private:
	wxChoice* m_pAttachments;

	wxPanel* m_pAttachmentInfo;

	wxStaticText* m_pName;
	wxStaticText* m_pType;
	wxStaticText* m_pBone;
	wxStaticText* m_pOrigin;
	wxStaticText* m_pVectors[ STUDIO_ATTACH_NUM_VECTORS ];

private:
	CAttachmentsPanel( const CAttachmentsPanel& ) = delete;
	CAttachmentsPanel& operator=( const CAttachmentsPanel& ) = delete;
};
}

#endif //HLMV_UI_CONTROLPANELS_CATTACHMENTSPANEL_H