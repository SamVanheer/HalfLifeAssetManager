#ifndef CONTROLPANELS_CTEXTURESPANEL_H
#define CONTROLPANELS_CTEXTURESPANEL_H

#include "CBaseControlPanel.h"

//Defined in a Windows header
#ifdef TRANSPARENT
#undef TRANSPARENT
#endif

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
	CTexturesPanel( wxWindow* pParent );
	~CTexturesPanel();

	void PanelActivated() override;

	void PanelDeactivated() override;

	void ModelChanged( const StudioModel& model ) override;

private:
	wxStaticText* m_pTextureSize;
	wxComboBox* m_pTexture;

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

#endif //CONTROLPANELS_CTEXTURESPANEL_H