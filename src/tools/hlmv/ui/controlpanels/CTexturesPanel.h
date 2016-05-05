#ifndef CONTROLPANELS_CTEXTURESPANEL_H
#define CONTROLPANELS_CTEXTURESPANEL_H

#include "ui/wx/utility/CMeshClientData.h"

#include "CBaseControlPanel.h"

//Defined in a Windows header
#ifdef TRANSPARENT
#undef TRANSPARENT
#endif

namespace hlmv
{
class CTexturesPanel final : public CBaseControlPanel
{
public:
	static const size_t TEXTUREVIEW_SLIDER_MIN = 1;
	static const size_t TEXTUREVIEW_SLIDER_MAX = 4;
	static const size_t TEXTUREVIEW_SLIDER_DEFAULT = TEXTUREVIEW_SLIDER_MIN;

	static const size_t NUM_CHECKBOXES_PER_ROW = 2;

	class CheckBox final
	{
	public:
		enum Type
		{
			FIRST				= 0,
			CHROME				= FIRST,
			SHOW_UV_MAP,
			ADDITIVE,
			OVERLAY_UV_MAP,
			TRANSPARENT,
			ANTI_ALIAS_LINES,

			COUNT,
			LAST				= COUNT - 1	//Must be last
		};

	private:
		CheckBox() = delete;
		CheckBox( const CheckBox& ) = delete;
		CheckBox& operator=( const CheckBox& ) = delete;
	};

public:
	CTexturesPanel( wxWindow* pParent, CHLMV* const pHLMV );
	~CTexturesPanel();

	MouseOpFlags_t GetAllowedMouseOperations() const override final { return MOUSEOPF_NONE; }

	void PanelActivated() override;

	void PanelDeactivated() override;

	void InitializeUI() override;

	void TextureChanged( wxCommandEvent& event );

	void ScaleChanged( wxCommandEvent& event );

	void CheckBoxChanged( wxCommandEvent& event );

	void MeshChanged( wxCommandEvent& event );

	void ImportTexture( wxCommandEvent& event );

	void ExportTexture( wxCommandEvent& event );

	void ExportUVMap( wxCommandEvent& event );

	void SetTexture( int iIndex );

	void SetScale( int iScale, const bool bSetSlider = true );

protected:
	wxDECLARE_EVENT_TABLE();

private:
	wxStaticText* m_pTextureSize;
	wxComboBox* m_pTexture;

	wxStaticText* m_pScaleTextureViewSize;
	wxSlider* m_pScaleTextureView;

	wxCheckBox* m_pCheckBoxes[ CheckBox::COUNT ];

	wxComboBox* m_pMesh;

	wxButton* m_pImportTexButton;
	wxButton* m_pExportTexButton;
	wxButton* m_pExportUVButton;

private:
	CTexturesPanel( const CTexturesPanel& ) = delete;
	CTexturesPanel& operator=( const CTexturesPanel& ) = delete;
};
}

#endif //CONTROLPANELS_CTEXTURESPANEL_H