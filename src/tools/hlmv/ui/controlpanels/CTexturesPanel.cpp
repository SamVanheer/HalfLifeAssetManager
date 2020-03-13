#include <new>
#include <memory>

#include <wx/filename.h>
#include <wx/gbsizer.h>
#include <wx/image.h>

#include "wx/utility/wxUtil.h"

#include "graphics/GraphicsHelpers.h"
#include "graphics/GraphicsUtils.h"
#include "graphics/Palette.h"
#include "graphics/BMPFile.h"

#include "shared/studiomodel/CStudioModel.h"

#include "../CModelViewerApp.h"
#include "../../CHLMVState.h"

#include "CTexturesPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CTexturesPanel, CBaseControlPanel )
	EVT_CHOICE( wxID_TEX_CHANGED, CTexturesPanel::TextureChanged )
	EVT_SLIDER( wxID_TEX_SCALE, CTexturesPanel::ScaleChanged )
	EVT_CHECKBOX( wxID_TEX_CHECKBOX, CTexturesPanel::CheckBoxChanged )
	EVT_CHOICE( wxID_TEX_MESH, CTexturesPanel::MeshChanged )
	EVT_BUTTON( wxID_TEX_IMPORTTEXTURE, CTexturesPanel::ImportTexture )
	EVT_BUTTON(wxID_TEX_IMPORTALLTEXTURES, CTexturesPanel::ImportAllTextures)
	EVT_BUTTON( wxID_TEX_EXPORTTEXTURE, CTexturesPanel::ExportTexture )
	EVT_BUTTON(wxID_TEX_EXPORTALLTEXTURES, CTexturesPanel::ExportAllTextures)
	EVT_BUTTON( wxID_TEX_EXPORTUVMAP, CTexturesPanel::ExportUVMap )
wxEND_EVENT_TABLE()

CTexturesPanel::CTexturesPanel( wxWindow* pParent, CModelViewerApp* const pHLMV )
	: CBaseControlPanel( pParent, "Textures", pHLMV )
{
	//Helps catch errors if we miss one.
	memset( m_pCheckBoxes, 0, sizeof( m_pCheckBoxes ) );

	wxWindow* const pElemParent = GetElementParent();

	m_pTextureSize = new wxStaticText(pElemParent, wxID_ANY, "Texture (size: Undefined x Undefined)" );

	m_pTexture = new wxChoice(pElemParent, wxID_TEX_CHANGED, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );

	m_pScaleTextureViewSize = new wxStaticText(pElemParent, wxID_ANY, "Scale Texture View (Undefinedx)" );

	m_pScaleTextureView = new wxSlider(pElemParent, wxID_TEX_SCALE, TEXTUREVIEW_SLIDER_DEFAULT, TEXTUREVIEW_SLIDER_MIN, TEXTUREVIEW_SLIDER_MAX );

	m_pCheckBoxes[ CheckBox::CHROME ]				= new wxCheckBox( pElemParent, wxID_TEX_CHECKBOX, "Chrome" );
	m_pCheckBoxes[ CheckBox::ADDITIVE ]				= new wxCheckBox( pElemParent, wxID_TEX_CHECKBOX, "Additive" );
	m_pCheckBoxes[ CheckBox::TRANSPARENT ]			= new wxCheckBox( pElemParent, wxID_TEX_CHECKBOX, "Transparent" );
	m_pCheckBoxes[ CheckBox::FULLBRIGHT ]			= new wxCheckBox( pElemParent, wxID_TEX_CHECKBOX, "Fullbright" );
	m_pCheckBoxes[ CheckBox::SHOW_UV_MAP ]			= new wxCheckBox( pElemParent, wxID_TEX_CHECKBOX, "Show UV Map" );
	m_pCheckBoxes[ CheckBox::OVERLAY_UV_MAP ]		= new wxCheckBox( pElemParent, wxID_TEX_CHECKBOX, "Overlay UV Map" );
	m_pCheckBoxes[ CheckBox::ANTI_ALIAS_LINES ]		= new wxCheckBox( pElemParent, wxID_TEX_CHECKBOX, "Anti-Alias Lines" );

	for( size_t uiIndex = CheckBox::FIRST; uiIndex < CheckBox::COUNT; ++uiIndex )
	{
		wxASSERT( m_pCheckBoxes[ uiIndex ] );

		m_pCheckBoxes[ uiIndex ]->SetClientData( reinterpret_cast<void*>( uiIndex ) );
	}

	m_pMesh = new wxChoice( pElemParent, wxID_TEX_MESH );

	wxPanel* buttonsPanel = new wxPanel(pElemParent);

	m_pImportTexButton = new wxButton(buttonsPanel, wxID_TEX_IMPORTTEXTURE, "Import Texture" );
	m_pImportAllTexturesButton = new wxButton(buttonsPanel, wxID_TEX_IMPORTALLTEXTURES, "Import All Textures");
	m_pExportTexButton = new wxButton(buttonsPanel, wxID_TEX_EXPORTTEXTURE, "Export Texture" );
	m_pExportAllTexturesButton = new wxButton(buttonsPanel, wxID_TEX_EXPORTALLTEXTURES, "Export All Textures");
	m_pExportUVButton = new wxButton(buttonsPanel, wxID_TEX_EXPORTUVMAP, "Export UV Map" );

	for (auto& slider : m_pColorSliders)
	{
		slider = new wxSlider(pElemParent, wxID_ANY, ColorDefault, ColorMin, ColorMax);
		slider->Bind(wxEVT_SLIDER, &CTexturesPanel::OnColorSliderChanged, this);
	}

	for (auto& spinner : m_pColorSpinners)
	{
		spinner = new wxSpinCtrl(pElemParent);
		spinner->SetRange(ColorMin, ColorMax);
		spinner->SetValue(ColorDefault);
		spinner->Bind(wxEVT_SPINCTRL, &CTexturesPanel::OnColorSpinnerChanged, this);
	}

	//Layout
	auto sizer = new wxGridBagSizer(1, 1);

	{
		auto textureSettingSizer = new wxBoxSizer(wxVERTICAL);

		textureSettingSizer->Add(m_pTextureSize, wxSizerFlags().Expand());
		textureSettingSizer->Add(m_pTexture, wxSizerFlags().Expand());
		textureSettingSizer->Add(m_pScaleTextureViewSize, wxSizerFlags().Expand());
		textureSettingSizer->Add(m_pScaleTextureView, wxSizerFlags().Expand());

		sizer->Add(textureSettingSizer, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	}

	sizer->Add(wx::CreateCheckBoxSizer(m_pCheckBoxes, ARRAYSIZE(m_pCheckBoxes), NUM_CHECKBOX_COLS, wxEXPAND), wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);

	sizer->Add(m_pMesh, wxGBPosition(0, 2), wxGBSpan(1, 1));

	{
		auto buttonsSizer = new wxGridBagSizer(1, 1);

		buttonsSizer->Add(m_pImportTexButton, wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
		buttonsSizer->Add(m_pImportAllTexturesButton, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);
		buttonsSizer->Add(m_pExportTexButton, wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
		buttonsSizer->Add(m_pExportAllTexturesButton, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
		buttonsSizer->Add(m_pExportUVButton, wxGBPosition(2, 0), wxDefaultSpan, wxEXPAND);

		buttonsPanel->SetSizer(buttonsSizer);

		sizer->Add(buttonsPanel, wxGBPosition(0, 3), wxGBSpan(1, 1), wxEXPAND);
	}

	{
		auto colorsSizer = new wxGridBagSizer(1, 1);

		colorsSizer->Add(new wxStaticText(pElemParent, wxID_ANY, "Top Color"), wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
		colorsSizer->Add(m_pColorSliders[0], wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);
		colorsSizer->Add(m_pColorSpinners[0], wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND);

		colorsSizer->Add(new wxStaticText(pElemParent, wxID_ANY, "Bottom Color"), wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
		colorsSizer->Add(m_pColorSliders[1], wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
		colorsSizer->Add(m_pColorSpinners[1], wxGBPosition(1, 2), wxDefaultSpan, wxEXPAND);

		sizer->Add(colorsSizer, wxGBPosition(0, 4), wxGBSpan(1, 1), wxEXPAND);
	}

	GetMainSizer()->Add(sizer);
}

CTexturesPanel::~CTexturesPanel()
{
}

void CTexturesPanel::PanelActivated()
{
	m_pHLMV->GetState()->showTexture = true;
}

void CTexturesPanel::PanelDeactivated()
{
	m_pHLMV->GetState()->showTexture = false;
}

void CTexturesPanel::InitializeUI()
{
	m_pTexture->Clear();

	m_pMesh->Clear();

	bool bSuccess = false;

	if( auto pEntity = m_pHLMV->GetState()->GetEntity() )
	{
		if( auto pModel = pEntity->GetModel() )
		{
			const studiohdr_t* const pHdr = pModel->GetTextureHeader();

			if( pHdr )
			{
				m_pTexture->Enable( true );

				const mstudiotexture_t* const pTextures = ( mstudiotexture_t* ) ( ( byte* ) pHdr + pHdr->textureindex );

				//Insert all names into the array, then append the array to the combo box. This is much faster than appending each name to the combo box directly.
				wxArrayString names;

				for( int i = 0; i < pHdr->numtextures; ++i )
				{
					const mstudiotexture_t& texture = pTextures[ i ];

					names.Add( texture.name );
				}

				m_pTexture->Append( names );

				bSuccess = true;
			}
		}
	}

	SetTexture( 0 );

	SetScale( TEXTUREVIEW_SLIDER_DEFAULT );

	if( !bSuccess )
	{
		m_pTexture->Enable( false );
		m_pMesh->Enable( false );
	}

	for (auto slider : m_pColorSliders)
	{
		slider->SetValue(0);
		slider->Enable(bSuccess);
	}

	for (auto spinner : m_pColorSpinners)
	{
		spinner->SetValue(0);
		spinner->Enable(bSuccess);
	}

	this->Enable( bSuccess );

	if (bSuccess)
	{
		RemapTextures();
	}
}

void CTexturesPanel::SetTexture( int iIndex )
{
	auto pState = m_pHLMV->GetState();

	//Reset all of these no matter what.
	pState->iOldTextureXOffset = pState->iOldTextureYOffset = pState->iTextureXOffset = pState->iTextureYOffset = 0;

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
	{
		m_pTextureSize->SetLabelText( "Texture" );
		return;
	}

	auto pStudioModel = pEntity->GetModel();

	wxASSERT( pStudioModel );

	const studiohdr_t* const pHdr = pStudioModel->GetStudioHeader();
	const studiohdr_t* const pTexHdr = pStudioModel->GetTextureHeader();

	m_pMesh->Clear();

	if( !pHdr || !pTexHdr )
	{
		m_pMesh->Enable( false );
		return;
	}

	if( iIndex < 0 || iIndex >= pTexHdr->numtextures )
		iIndex = 0;

	const mstudiotexture_t& texture = ( ( mstudiotexture_t* ) ( ( byte* ) pTexHdr + pTexHdr->textureindex ) ) [ iIndex ];

	m_pTexture->Select( iIndex );

	m_pTextureSize->SetLabelText( wxString::Format( "Texture (size: %d x %d)", texture.width, texture.height ) );

	m_pCheckBoxes[ CheckBox::CHROME ]->SetValue( ( texture.flags & STUDIO_NF_CHROME ) != 0 );
	m_pCheckBoxes[ CheckBox::ADDITIVE ]->SetValue( ( texture.flags & STUDIO_NF_ADDITIVE ) != 0 );
	m_pCheckBoxes[ CheckBox::TRANSPARENT ]->SetValue( ( texture.flags & STUDIO_NF_MASKED ) != 0 );
	m_pCheckBoxes[ CheckBox::FULLBRIGHT ]->SetValue( ( texture.flags & STUDIO_NF_FULLBRIGHT ) != 0 );

	const CStudioModelEntity::MeshList_t meshes = pEntity->ComputeMeshList( iIndex );

	m_pMesh->Enable( true );

	size_t uiIndex;

	for( uiIndex = 0; uiIndex < meshes.size(); ++uiIndex )
	{
		m_pMesh->Append( wxString::Format( "Mesh %u", uiIndex + 1 ), new ui::CMeshClientData( meshes[ uiIndex ] ) );
	}

	if( uiIndex > 0 )
	{
		m_pHLMV->GetState()->pUVMesh = meshes[ 0 ];

		if( uiIndex > 1 )
		{
			m_pMesh->Append( "All" );
		}
	}
	else
	{
		m_pHLMV->GetState()->pUVMesh = nullptr;
	}

	m_pMesh->Select( 0 );

	m_pHLMV->GetState()->texture = iIndex;
}

void CTexturesPanel::SetScale( int iScale, const bool bSetSlider )
{
	iScale = clamp( iScale, static_cast<int>( TEXTUREVIEW_SLIDER_MIN ), static_cast<int>( TEXTUREVIEW_SLIDER_MAX ) );

	m_pHLMV->GetState()->textureScale = iScale;

	if( bSetSlider )
		m_pScaleTextureView->SetValue( iScale );

	m_pScaleTextureViewSize->SetLabelText( wxString::Format( "Scale Texture View (%dx)", iScale ) );
}

void CTexturesPanel::TextureChanged( wxCommandEvent& event )
{
	SetTexture( m_pTexture->GetSelection() );
}

void CTexturesPanel::ScaleChanged( wxCommandEvent& event )
{
	SetScale( m_pScaleTextureView->GetValue(), false );
}

void CTexturesPanel::CheckBoxChanged( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	auto pModel = pEntity->GetModel();

	if( !pModel )
		return;

	const studiohdr_t* const pHdr = pModel->GetTextureHeader();

	if( !pHdr )
		return;

	wxCheckBox* const pCheckBox = static_cast<wxCheckBox*>( event.GetEventObject() );

	const CheckBox::Type checkbox = static_cast<CheckBox::Type>( reinterpret_cast<int>( pCheckBox->GetClientData() ) );

	if( checkbox < CheckBox::FIRST || checkbox > CheckBox::LAST )
		return;

	mstudiotexture_t& texture = ( ( mstudiotexture_t* ) ( ( byte* ) pHdr + pHdr->textureindex ) )[ m_pTexture->GetSelection() ];

	switch( checkbox )
	{
	case CheckBox::CHROME:
		{
			//Chrome disables alpha testing.
			if( pCheckBox->GetValue() )
			{
				texture.flags &= ~STUDIO_NF_MASKED;
				m_pCheckBoxes[ CheckBox::TRANSPARENT ]->SetValue( false );
			}

			if( pCheckBox->GetValue() )
			{
				texture.flags |= STUDIO_NF_CHROME;
			}
			else
			{
				texture.flags &= ~STUDIO_NF_CHROME;
			}

			break;
		}

	case CheckBox::ADDITIVE:
		{
			//Additive disables alpha testing.
			if( pCheckBox->GetValue() )
			{
				texture.flags &= ~STUDIO_NF_MASKED;
				m_pCheckBoxes[ CheckBox::TRANSPARENT ]->SetValue( false );
			}

			if( pCheckBox->GetValue() )
			{
				texture.flags |= STUDIO_NF_ADDITIVE;
			}
			else
			{
				texture.flags &= ~STUDIO_NF_ADDITIVE;
			}

			break;
		}

	case CheckBox::TRANSPARENT:
		{
			//Alpha testing disables chrome and additive.
			if( pCheckBox->GetValue() )
			{
				texture.flags &= ~( STUDIO_NF_CHROME | STUDIO_NF_ADDITIVE );
				m_pCheckBoxes[ CheckBox::CHROME ]->SetValue( false );
				m_pCheckBoxes[ CheckBox::ADDITIVE ]->SetValue( false );
			}

			if( pCheckBox->GetValue() )
			{
				texture.flags |= STUDIO_NF_MASKED;
			}
			else
			{
				texture.flags &= ~STUDIO_NF_MASKED;
			}

			pModel->ReuploadTexture( &texture );

			break;
		}

	case CheckBox::FULLBRIGHT:
		{
			if( pCheckBox->GetValue() )
			{
				texture.flags |= STUDIO_NF_FULLBRIGHT;
			}
			else
			{
				texture.flags &= ~STUDIO_NF_FULLBRIGHT;
			}

			break;
		}

	case CheckBox::SHOW_UV_MAP:
		{
			m_pHLMV->GetState()->showUVMap = pCheckBox->GetValue();

			break;
		}

	case CheckBox::OVERLAY_UV_MAP:
		{
			m_pHLMV->GetState()->overlayUVMap = pCheckBox->GetValue();

			break;
		}

	case CheckBox::ANTI_ALIAS_LINES:
		{
			m_pHLMV->GetState()->antiAliasUVLines = pCheckBox->GetValue();

			break;
		}
	}

	switch( checkbox )
	{
	case CheckBox::CHROME:
	case CheckBox::ADDITIVE:
	case CheckBox::TRANSPARENT:
	case CheckBox::FULLBRIGHT:
		{
			m_pHLMV->GetState()->modelChanged = true;

			break;
		}

	default: break;
	}
}

void CTexturesPanel::MeshChanged( wxCommandEvent& event )
{
	const int iIndex = m_pMesh->GetSelection();

	if( iIndex == wxNOT_FOUND )
		return;

	const ui::CMeshClientData* pMesh = static_cast<const ui::CMeshClientData*>( m_pMesh->GetClientObject( iIndex ) );

	//Null client data means it's "All"
	m_pHLMV->GetState()->pUVMesh = pMesh ? pMesh->m_pMesh : nullptr;
}

void CTexturesPanel::ImportTextureFrom(const wxString& fileName, studiomdl::CStudioModel* pStudioModel, studiohdr_t* pHdr, int textureIndex)
{
	//Must be BMP
	wxImage image(fileName, wxBITMAP_TYPE_BMP);

	if (!image.IsOk())
	{
		wxMessageBox(wxString::Format("Failed to load image \"%s\"!", fileName.c_str()));
		return;
	}

	const wxPalette& palette = image.GetPalette();

	if (!palette.IsOk())
	{
		wxMessageBox(wxString::Format("Palette for image \"%s\" does not exist!", fileName.c_str()));
		return;
	}

	mstudiotexture_t& texture = ((mstudiotexture_t*) ((byte*) pHdr + pHdr->textureindex))[textureIndex];

	if (texture.width != image.GetWidth() || texture.height != image.GetHeight())
	{
		wxMessageBox(wxString::Format("Image \"%s\" does not have matching dimensions to the current texture (src: %d x %d, dest: %d x %d)",
			fileName.c_str(),
			image.GetWidth(), image.GetHeight(),
			texture.width, texture.height));
		return;
	}

	//Convert to 8 bit palette based image.
	std::unique_ptr<byte[]> texData = std::make_unique<byte[]>(image.GetWidth() * image.GetHeight());

	byte* pDest = texData.get();

	const unsigned char* pSourceData = image.GetData();

	for (int i = 0; i < image.GetWidth() * image.GetHeight(); ++i, ++pDest, pSourceData += 3)
	{
		*pDest = palette.GetPixel(pSourceData[0], pSourceData[1], pSourceData[2]);
	}

	byte convPal[PALETTE_SIZE];

	memset(convPal, 0, sizeof(convPal));

	unsigned char r, g, b;

	for (size_t uiIndex = 0; uiIndex < PALETTE_ENTRIES; ++uiIndex)
	{
		if (palette.GetRGB(uiIndex, &r, &g, &b))
		{
			convPal[uiIndex * PALETTE_CHANNELS] = r;
			convPal[uiIndex * PALETTE_CHANNELS + 1] = g;
			convPal[uiIndex * PALETTE_CHANNELS + 2] = b;
		}
	}

	//Copy over the new image data to the texture.
	memcpy((byte*) pHdr + texture.index, texData.get(), image.GetWidth() * image.GetHeight());
	memcpy((byte*) pHdr + texture.index + image.GetWidth() * image.GetHeight(), convPal, PALETTE_SIZE);

	pStudioModel->ReplaceTexture(&texture, texData.get(), convPal, pStudioModel->GetTextureId(textureIndex));

	m_pHLMV->GetState()->modelChanged = true;
}

void CTexturesPanel::ImportTexture( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity || !pEntity->GetModel() )
	{
		wxMessageBox( "No model loaded!" );
		return;
	}

	auto pStudioModel = pEntity->GetModel();

	const int iTextureIndex = m_pTexture->GetSelection();

	if( iTextureIndex == wxNOT_FOUND )
	{
		wxMessageBox( "No texture selected" );
		return;
	}

	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Windows Bitmap (*.bmp)|*.bmp" );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szFilename = dlg.GetPath();

	ImportTextureFrom(szFilename, pStudioModel, pStudioModel->GetTextureHeader(), iTextureIndex);

	RemapTexture(iTextureIndex);
}

void CTexturesPanel::ImportAllTextures(wxCommandEvent& event)
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if (!pEntity || !pEntity->GetModel())
	{
		wxMessageBox("No model loaded!");
		return;
	}

	wxDirDialog dlg(this, "Select the directory to import all textures from", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_NEW_DIR_BUTTON);

	if (dlg.ShowModal() == wxID_CANCEL)
	{
		return;
	}

	auto pStudioModel = pEntity->GetModel();
	studiohdr_t* const pHdr = pStudioModel->GetTextureHeader();

	//For each texture in the model, find if there is a file with the same name in the given directory
	//If so, try to replace the texture
	for (int i = 0; i < pHdr->numtextures; ++i)
	{
		mstudiotexture_t& texture = ((mstudiotexture_t*) ((byte*) pHdr + pHdr->textureindex))[i];

		auto fileName{wxFileName::DirName(dlg.GetPath())};

		fileName.SetName(texture.name);

		if (fileName.FileExists())
		{
			ImportTextureFrom(fileName.GetFullPath(), pStudioModel, pHdr, i);
		}
	}

	RemapTextures();
}

void CTexturesPanel::ExportTexture( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity || !pEntity->GetModel() )
	{
		wxMessageBox( "No model loaded!" );
		return;
	}

	auto pStudioModel = pEntity->GetModel();

	const int iTextureIndex = m_pTexture->GetSelection();

	if( iTextureIndex == wxNOT_FOUND )
	{
		wxMessageBox( "No texture selected" );
		return;
	}

	studiohdr_t* const pHdr = pStudioModel->GetTextureHeader();

	mstudiotexture_t& texture = ((mstudiotexture_t*) ((byte*) pHdr + pHdr->textureindex))[iTextureIndex];

	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, texture.name, "Windows Bitmap (*.bmp)|*.bmp", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szFilename = dlg.GetPath();

	if( !graphics::bmpfile::SaveBMPFile( szFilename.c_str(), texture.width, texture.height,
		( uint8_t* ) pHdr + texture.index, ( uint8_t* ) pHdr + texture.index + texture.width * texture.height ) )
	{
		wxMessageBox( wxString::Format( "Failed to save image \"%s\"!", szFilename.c_str() ) );
	}
}

void CTexturesPanel::ExportAllTextures(wxCommandEvent& event)
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if (!pEntity || !pEntity->GetModel())
	{
		wxMessageBox("No model loaded!");
		return;
	}

	auto pStudioModel = pEntity->GetModel();

	wxDirDialog dlg(this, "Select the directory to export all textures to", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_NEW_DIR_BUTTON);

	if (dlg.ShowModal() == wxID_CANCEL)
	{
		return;
	}

	studiohdr_t* const pHdr = pStudioModel->GetTextureHeader();

	wxString errors;

	for (int i = 0; i < pHdr->numtextures; ++i)
	{
		const auto& texture = ((mstudiotexture_t*) ((byte*) pHdr + pHdr->textureindex))[i];

		auto fileName{wxFileName::DirName(dlg.GetPath())};

		fileName.SetName(texture.name);

		auto fullPath = fileName.GetFullPath();

		if (!graphics::bmpfile::SaveBMPFile(fullPath.c_str(), texture.width, texture.height,
			(uint8_t*) pHdr + texture.index, (uint8_t*) pHdr + texture.index + texture.width * texture.height))
		{
			errors += wxString::Format("\"%s\"\n", fullPath.c_str());
		}
	}

	if (!errors.empty())
	{
		wxMessageBox(wxString::Format("Failed to save images:\n%s", errors.c_str()));
	}
}

void CTexturesPanel::ExportUVMap( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity || !pEntity->GetModel() )
	{
		wxMessageBox( "No model loaded!" );
		return;
	}

	auto pStudioModel = pEntity->GetModel();

	const int iTextureIndex = m_pTexture->GetSelection();

	if( iTextureIndex == wxNOT_FOUND )
	{
		wxMessageBox( "No texture selected" );
		return;
	}

	wxFileDialog dlg( this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Windows Bitmap (*.bmp)|*.bmp", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

	if( dlg.ShowModal() == wxID_CANCEL )
		return;

	const wxString szFilename = dlg.GetPath();

	m_pHLMV->SaveUVMap( szFilename, iTextureIndex );
}

void CTexturesPanel::OnColorSliderChanged(wxCommandEvent& event)
{
	if (auto entity = m_pHLMV->GetState()->GetEntity(); entity)
	{
		const auto index = event.GetEventObject() == m_pColorSliders[0] ? 0 : 1;

		m_pColorSpinners[index]->SetValue(m_pColorSliders[index]->GetValue());

		RemapTextures();
	}
}

void CTexturesPanel::OnColorSpinnerChanged(wxSpinEvent& event)
{
	if (auto entity = m_pHLMV->GetState()->GetEntity(); entity)
	{
		const auto index = event.GetEventObject() == m_pColorSpinners[0] ? 0 : 1;

		m_pColorSliders[index]->SetValue(m_pColorSpinners[index]->GetValue());

		RemapTextures();
	}
}

void CTexturesPanel::RemapTextures()
{
	if (auto entity = m_pHLMV->GetState()->GetEntity(); entity)
	{
		auto textureHeader = entity->GetModel()->GetTextureHeader();

		for (int i = 0; i < textureHeader->numtextures; ++i)
		{
			RemapTexture(i);
		}
	}
}

void CTexturesPanel::RemapTexture(int index)
{
	auto entity = m_pHLMV->GetState()->GetEntity();

	auto textureHeader = entity->GetModel()->GetTextureHeader();

	const auto texture = textureHeader->GetTexture(index);

	const auto textureId = entity->GetModel()->GetTextureId(index);

	int low, mid, high;

	if (graphics::TryGetRemapColors(texture->name, low, mid, high))
	{
		byte palette[PALETTE_SIZE];

		memcpy(palette, reinterpret_cast<byte*>(textureHeader) + texture->index + texture->width * texture->height, PALETTE_SIZE);

		graphics::PaletteHueReplace(palette, m_pColorSliders[0]->GetValue(), low, mid - 1);

		if (high)
		{
			graphics::PaletteHueReplace(palette, m_pColorSliders[1]->GetValue(), mid, high);
		}

		entity->GetModel()->ReplaceTexture(texture, reinterpret_cast<byte*>(textureHeader) + texture->index, palette, textureId);
	}
}
}