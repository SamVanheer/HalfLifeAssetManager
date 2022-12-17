#include <algorithm>
#include <utility>
#include <vector>

#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QMetaEnum>
#include <QSignalBlocker>
#include <QToolTip>

#include "entity/HLMVStudioModelEntity.hpp"
#include "entity/TextureEntity.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/Palette.hpp"
#include "graphics/Scene.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/StateSnapshot.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelTextureUtilities.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"
#include "ui/assets/studiomodel/StudioModelValidators.hpp"
#include "ui/assets/studiomodel/dockpanels/ExportUVMeshDialog.hpp"
#include "ui/assets/studiomodel/dockpanels/TexturesPanel.hpp"

#include "ui/camera_operators/TextureCameraOperator.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace studiomodel
{
static constexpr double TextureViewScaleMinimum = 0.1;
static constexpr double TextureViewScaleMaximum = 20;
static constexpr double TextureViewScaleDefault = 1;
static constexpr double TextureViewScaleSingleStepValue = 0.1;
static constexpr double TextureViewScaleSliderRatio = 10.0;
static constexpr double UVLineWidthSliderRatio = 10.0;

static int GetMeshIndexForDrawing(QComboBox* comboBox)
{
	int meshIndex = comboBox->currentIndex();

	if (comboBox->count() > 1 && meshIndex == (comboBox->count() - 1))
	{
		meshIndex = -1;
	}

	return meshIndex;
}

TexturesPanel::TexturesPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	const auto textureNameValidator = new UniqueTextureNameValidator(MaxTextureNameBytes - 1, _asset, this);

	_ui.TextureName->setValidator(textureNameValidator);

	const auto studioModelSettings{_asset->GetProvider()->GetStudioModelSettings()};

	_ui.ScaleTextureViewSpinner->setRange(TextureViewScaleMinimum, TextureViewScaleMaximum);
	_ui.ScaleTextureViewSpinner->setValue(TextureViewScaleDefault);
	_ui.ScaleTextureViewSpinner->setSingleStep(TextureViewScaleSingleStepValue);

	_ui.ScaleTextureViewSlider->setRange(
		0,
		static_cast<int>((_ui.ScaleTextureViewSpinner->maximum() - _ui.ScaleTextureViewSpinner->minimum()) * TextureViewScaleSliderRatio));

	_ui.ScaleTextureViewSlider->setValue(
		static_cast<int>((_ui.ScaleTextureViewSpinner->value() - _ui.ScaleTextureViewSpinner->minimum()) * TextureViewScaleSliderRatio));

	_ui.UVLineWidthSlider->setRange(
		static_cast<int>(_ui.UVLineWidthSpinner->minimum() * UVLineWidthSliderRatio),
		static_cast<int>(_ui.UVLineWidthSpinner->maximum() * UVLineWidthSliderRatio));

	_ui.UVLineWidthSlider->setValue(static_cast<int>(_ui.UVLineWidthSpinner->value() * UVLineWidthSliderRatio));

	connect(_ui.Textures, qOverload<int>(&QComboBox::currentIndexChanged), textureNameValidator, &UniqueTextureNameValidator::SetCurrentIndex);

	connect(_asset, &StudioModelAsset::AssetChanged, this, &TexturesPanel::OnAssetChanged);
	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &TexturesPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &TexturesPanel::OnLoadSnapshot);

	connect(_asset->GetTextureCameraOperator(), &TextureCameraOperator::ScaleChanged, this, &TexturesPanel::OnScaleChanged);

	connect(_ui.Textures, qOverload<int>(&QComboBox::currentIndexChanged), this, &TexturesPanel::OnTextureChanged);
	connect(_ui.ScaleTextureViewSlider, &QSlider::valueChanged, this, &TexturesPanel::OnTextureViewScaleSliderChanged);
	connect(_ui.ScaleTextureViewSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TexturesPanel::OnTextureViewScaleSpinnerChanged);
	connect(_ui.UVLineWidthSlider, &QSlider::valueChanged, this, &TexturesPanel::OnUVLineWidthSliderChanged);
	connect(_ui.UVLineWidthSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TexturesPanel::OnUVLineWidthSpinnerChanged);

	connect(_ui.TextureName, &QLineEdit::textChanged, this, &TexturesPanel::OnTextureNameChanged);
	connect(_ui.TextureName, &QLineEdit::inputRejected, this, &TexturesPanel::OnTextureNameRejected);

	connect(_ui.Chrome, &QCheckBox::stateChanged, this, &TexturesPanel::OnChromeChanged);
	connect(_ui.Additive, &QCheckBox::stateChanged, this, &TexturesPanel::OnAdditiveChanged);
	connect(_ui.Transparent, &QCheckBox::stateChanged, this, &TexturesPanel::OnTransparentChanged);
	connect(_ui.FlatShade, &QCheckBox::stateChanged, this, &TexturesPanel::OnFlatShadeChanged);
	connect(_ui.Fullbright, &QCheckBox::stateChanged, this, &TexturesPanel::OnFullbrightChanged);
	connect(_ui.Mipmaps, &QCheckBox::stateChanged, this, &TexturesPanel::OnMipmapsChanged);

	connect(_ui.ShowUVMap, &QCheckBox::stateChanged, this, &TexturesPanel::UpdateUVMapTexture);
	connect(_ui.OverlayUVMap, &QCheckBox::stateChanged, this, &TexturesPanel::OnOverlayUVMapChanged);
	connect(_ui.AntiAliasLines, &QCheckBox::stateChanged, this, &TexturesPanel::UpdateUVMapTexture);

	connect(_ui.Meshes, qOverload<int>(&QComboBox::currentIndexChanged), this, &TexturesPanel::UpdateUVMapTexture);

	connect(_ui.ImportTexture, &QPushButton::clicked, this, &TexturesPanel::OnImportTexture);
	connect(_ui.ExportTexture, &QPushButton::clicked, this, &TexturesPanel::OnExportTexture);
	connect(_ui.ExportUVMap, &QPushButton::clicked, this, &TexturesPanel::OnExportUVMap);

	connect(_ui.ImportAllTextures, &QPushButton::clicked, this, &TexturesPanel::OnImportAllTextures);
	connect(_ui.ExportAllTextures, &QPushButton::clicked, this, &TexturesPanel::OnExportAllTextures);

	connect(_ui.TopColorSlider, &QSlider::valueChanged, this, &TexturesPanel::OnTopColorSliderChanged);
	connect(_ui.BottomColorSlider, &QSlider::valueChanged, this, &TexturesPanel::OnBottomColorSliderChanged);
	connect(_ui.TopColorSpinner, qOverload<int>(&QSpinBox::valueChanged), this, &TexturesPanel::OnTopColorSpinnerChanged);
	connect(_ui.BottomColorSpinner, qOverload<int>(&QSpinBox::valueChanged), this, &TexturesPanel::OnBottomColorSpinnerChanged);

	OnAssetChanged(nullptr);
}

TexturesPanel::~TexturesPanel() = default;

void TexturesPanel::OnVisibilityChanged(bool visible)
{
	if (_asset->GetProvider()->GetStudioModelSettings()->ShouldActivateTextureViewWhenTexturesPanelOpened())
	{
		auto scene = visible ? _asset->GetTextureScene() : _asset->GetScene();

		_asset->SetCurrentScene(scene);
	}
}

static void SetTextureFlagCheckBoxes(Ui_TexturesPanel& ui, int flags)
{
	const QSignalBlocker chrome{ui.Chrome};
	const QSignalBlocker additive{ui.Additive};
	const QSignalBlocker transparent{ui.Transparent};
	const QSignalBlocker flatShade{ui.FlatShade};
	const QSignalBlocker fullbright{ui.Fullbright};
	const QSignalBlocker nomipmaps{ui.Mipmaps};

	ui.Chrome->setChecked((flags & STUDIO_NF_CHROME) != 0);
	ui.Additive->setChecked((flags & STUDIO_NF_ADDITIVE) != 0);
	ui.Transparent->setChecked((flags & STUDIO_NF_MASKED) != 0);
	ui.FlatShade->setChecked((flags & STUDIO_NF_FLATSHADE) != 0);
	ui.Fullbright->setChecked((flags & STUDIO_NF_FULLBRIGHT) != 0);

	//TODO: change the constant name to reflect the actual behavior (flag enables mipmaps)
	ui.Mipmaps->setChecked((flags & STUDIO_NF_NOMIPS) != 0);
}

void TexturesPanel::OnAssetChanged(StudioModelAsset* asset)
{
	auto modelData = asset ? asset->GetModelData() : StudioModelData::GetEmptyModel();

	_ui.Textures->setModel(modelData->Textures);

	this->setEnabled(_ui.Textures->count() > 0);

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	connect(modelData, &StudioModelData::TextureNameChanged, this, [this](int index)
		{
			if (index == _ui.Textures->currentIndex())
			{
				const QString name{_asset->GetEditableStudioModel()->Textures[index]->Name.c_str()};

				//Avoid resetting the edit position
				if (_ui.TextureName->text() != name)
				{
					const QSignalBlocker blocker{_ui.TextureName};
					_ui.TextureName->setText(name);
				}
			}
		});

	connect(modelData, &StudioModelData::TextureFlagsChanged, this, [this](int index)
		{
			if (index == _ui.Textures->currentIndex())
			{
				SetTextureFlagCheckBoxes(_ui, _asset->GetEditableStudioModel()->Textures[index]->Flags);
			}
		});
}

void TexturesPanel::OnSaveSnapshot(StateSnapshot* snapshot)
{
	if (auto index = _ui.Textures->currentIndex(); index != -1)
	{
		auto model = _asset->GetEntity()->GetEditableModel();

		const auto& texture = *model->Textures[index];

		snapshot->SetValue("textures.texture", QString::fromStdString(texture.Name));
	}
}

void TexturesPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	if (auto texture = snapshot->Value("textures.texture"); texture.isValid())
	{
		auto textureName = texture.toString().toStdString();

		auto model = _asset->GetEntity()->GetEditableModel();

		if (auto it = std::find_if(model->Textures.begin(), model->Textures.end(), [&](const auto& texture)
			{
				return texture->Name == textureName;
			}); it != model->Textures.end())
		{
			const auto index = it - model->Textures.begin();

			_ui.Textures->setCurrentIndex(index);
		}
	}
}

void TexturesPanel::OnScaleChanged(float adjust)
{
	_ui.ScaleTextureViewSpinner->setValue(_ui.ScaleTextureViewSpinner->value() + adjust);
}

void TexturesPanel::OnTextureChanged(int index)
{
	auto entity = _asset->GetEntity();
	auto textureEntity = _asset->GetTextureEntity();

	//Reset texture position to be centered
	textureEntity->XOffset = textureEntity->YOffset = 0;
	textureEntity->TextureIndex = index;

	_ui.Meshes->clear();
	_ui.Meshes->setEnabled(index != -1);

	const studiomdl::Texture emptyTexture{};

	const auto& texture = index != -1 ? *entity->GetEditableModel()->Textures[index] : emptyTexture;

	{
		const QSignalBlocker name{_ui.TextureName};

		_ui.TextureName->setText(texture.Name.c_str());
	}

	SetTextureFlagCheckBoxes(_ui, texture.Flags);

	const auto meshes = entity->GetEditableModel()->ComputeMeshList(index);

	for (decltype(meshes.size()) i = 0; i < meshes.size(); ++i)
	{
		_ui.Meshes->addItem(QString{"Mesh %1"}.arg(i + 1));
	}

	if (_ui.Meshes->count() > 1)
	{
		_ui.Meshes->addItem("All");
	}

	UpdateUVMapTexture();
}

void TexturesPanel::OnTextureViewScaleSliderChanged(int value)
{
	const double newValue = (value / UVLineWidthSliderRatio) + _ui.ScaleTextureViewSpinner->minimum();

	{
		const QSignalBlocker blocker{_ui.ScaleTextureViewSpinner};
		_ui.ScaleTextureViewSpinner->setValue(newValue);
	}

	_asset->GetTextureEntity()->TextureScale = newValue;

	UpdateUVMapTexture();
}

void TexturesPanel::OnTextureViewScaleSpinnerChanged(double value)
{
	{
		const QSignalBlocker blocker{_ui.ScaleTextureViewSlider};
		_ui.ScaleTextureViewSlider->setValue(static_cast<int>((value - _ui.ScaleTextureViewSpinner->minimum()) * UVLineWidthSliderRatio));
	}

	_asset->GetTextureEntity()->TextureScale = value;

	UpdateUVMapTexture();
}

void TexturesPanel::OnUVLineWidthSliderChanged(int value)
{
	const double newValue = value / UVLineWidthSliderRatio;

	{
		const QSignalBlocker blocker{_ui.UVLineWidthSpinner};
		_ui.UVLineWidthSpinner->setValue(newValue);
	}

	UpdateUVMapTexture();
}

void TexturesPanel::OnUVLineWidthSpinnerChanged(double value)
{
	{
		const QSignalBlocker blocker{_ui.UVLineWidthSlider};
		_ui.UVLineWidthSlider->setValue(static_cast<int>(value * UVLineWidthSliderRatio));
	}

	UpdateUVMapTexture();
}

void TexturesPanel::OnTextureNameChanged()
{
	const auto& texture = *_asset->GetEntity()->GetEditableModel()->Textures[_ui.Textures->currentIndex()];

	_asset->AddUndoCommand(new ChangeTextureNameCommand(_asset, _ui.Textures->currentIndex(), texture.Name.c_str(), _ui.TextureName->text()));
}

void TexturesPanel::OnTextureNameRejected()
{
	QToolTip::showText(_ui.TextureName->mapToGlobal({0, -20}), "Texture names must be unique");
}

void TexturesPanel::OnChromeChanged()
{
	const auto& texture = *_asset->GetEntity()->GetEditableModel()->Textures[_ui.Textures->currentIndex()];

	int flags = texture.Flags;

	//Chrome disables alpha testing
	if (_ui.Chrome->isChecked())
	{
		flags = SetFlags(flags, STUDIO_NF_MASKED, false);
	}

	flags = SetFlags(flags, STUDIO_NF_CHROME, _ui.Chrome->isChecked());

	_asset->AddUndoCommand(new ChangeTextureFlagsCommand(_asset, _ui.Textures->currentIndex(), texture.Flags, flags));
}

void TexturesPanel::OnAdditiveChanged()
{
	const auto& texture = *_asset->GetEntity()->GetEditableModel()->Textures[_ui.Textures->currentIndex()];

	int flags = texture.Flags;

	//Additive disables alpha testing
	if (_ui.Additive->isChecked())
	{
		flags = SetFlags(flags, STUDIO_NF_MASKED, false);
	}

	flags = SetFlags(flags, STUDIO_NF_ADDITIVE, _ui.Additive->isChecked());

	_asset->AddUndoCommand(new ChangeTextureFlagsCommand(_asset, _ui.Textures->currentIndex(), texture.Flags, flags));
}

void TexturesPanel::OnTransparentChanged()
{
	const auto& texture = *_asset->GetEntity()->GetEditableModel()->Textures[_ui.Textures->currentIndex()];

	int flags = texture.Flags;

	//Alpha testing disables chrome and additive
	if (_ui.Transparent->isChecked())
	{
		flags = SetFlags(flags, STUDIO_NF_CHROME | STUDIO_NF_ADDITIVE, false);
	}

	flags = SetFlags(flags, STUDIO_NF_MASKED, _ui.Transparent->isChecked());

	_asset->AddUndoCommand(new ChangeTextureFlagsCommand(_asset, _ui.Textures->currentIndex(), texture.Flags, flags));
}

void TexturesPanel::OnFlatShadeChanged()
{
	const auto& texture = *_asset->GetEntity()->GetEditableModel()->Textures[_ui.Textures->currentIndex()];

	const int flags = SetFlags(texture.Flags, STUDIO_NF_FLATSHADE, _ui.FlatShade->isChecked());

	_asset->AddUndoCommand(new ChangeTextureFlagsCommand(_asset, _ui.Textures->currentIndex(), texture.Flags, flags));
}

void TexturesPanel::OnFullbrightChanged()
{
	const auto& texture = *_asset->GetEntity()->GetEditableModel()->Textures[_ui.Textures->currentIndex()];

	const int flags = SetFlags(texture.Flags, STUDIO_NF_FULLBRIGHT, _ui.Fullbright->isChecked());

	_asset->AddUndoCommand(new ChangeTextureFlagsCommand(_asset, _ui.Textures->currentIndex(), texture.Flags, flags));
}

void TexturesPanel::OnMipmapsChanged()
{
	const auto& texture = *_asset->GetEntity()->GetEditableModel()->Textures[_ui.Textures->currentIndex()];

	const int flags = SetFlags(texture.Flags, STUDIO_NF_NOMIPS, _ui.Mipmaps->isChecked());

	_asset->AddUndoCommand(new ChangeTextureFlagsCommand(_asset, _ui.Textures->currentIndex(), texture.Flags, flags));
}

void TexturesPanel::OnOverlayUVMapChanged()
{
	_asset->GetTextureEntity()->OverlayUVMap = _ui.OverlayUVMap->isChecked();
}

void TexturesPanel::ImportTextureFrom(const QString& fileName, studiomdl::EditableStudioModel& model, int textureIndex)
{
	QImage image{fileName};

	if (image.isNull())
	{
		QMessageBox::critical(this, "Error loading image", QString{"Failed to load image \"%1\"."}.arg(fileName));
		return;
	}

	auto convertedTexture = ConvertImageToTexture(image);

	if (!convertedTexture)
	{
		QMessageBox::critical(this, "Error loading image", QString{"Palette for image \"%1\" does not exist."}.arg(fileName));
		return;
	}

	if (std::get<1>(convertedTexture.value()))
	{
		QMessageBox::warning(this, "Warning",
			QString{"Image \"%1\" has the format \"%2\" and will be converted to an indexed 8 bit image. Loss of color depth may occur."}
			.arg(fileName)
			.arg(QMetaEnum::fromType<QImage::Format>().valueToKey(image.format())));
	}

	auto& textureData = std::get<0>(convertedTexture.value());

	auto& texture = *model.Textures[textureIndex];

	auto scaledSTCoordinates = studiomdl::CalculateScaledSTCoordinatesData(
		model, textureIndex, texture.Data.Width, texture.Data.Height, image.width(), image.height());

	ImportTextureData oldTexture;
	ImportTextureData newTexture;

	oldTexture.Data = texture.Data;
	oldTexture.ScaledSTCoordinates = std::move(scaledSTCoordinates.first);

	newTexture.Data = textureData;
	newTexture.ScaledSTCoordinates = std::move(scaledSTCoordinates.second);

	_asset->AddUndoCommand(new ImportTextureCommand(_asset, textureIndex, std::move(oldTexture), std::move(newTexture)));
}

void TexturesPanel::UpdateColormapValue()
{
	const auto topColor = _ui.TopColorSlider->value();
	const auto bottomColor = _ui.BottomColorSlider->value();

	const int colormapValue = topColor & 0xFF | ((bottomColor & 0xFF) << 8);

	_ui.ColormapValue->setText(QString::number(colormapValue));

	auto model = _asset->GetEditableStudioModel();

	model->TopColor = topColor;
	model->BottomColor = bottomColor;

	auto graphicsContext = _asset->GetScene()->GetGraphicsContext();

	graphicsContext->Begin();
	model->UpdateTextures(*_asset->GetTextureLoader());
	graphicsContext->End();
}

void TexturesPanel::UpdateUVMapTexture()
{
	auto textureEntity = _asset->GetTextureEntity();

	auto scene = _asset->GetScene();

	textureEntity->ShowUVMap = _ui.ShowUVMap->isChecked();

	if (!_ui.ShowUVMap->isChecked())
	{
		return;
	}

	const int textureIndex = _ui.Textures->currentIndex();

	if (textureIndex == -1)
	{
		//Make sure nothing is drawn when no texture is selected
		std::byte transparentImage[4];

		std::fill_n(transparentImage, std::size(transparentImage), std::byte{0});

		textureEntity->SetUVMeshImage({1, 1, transparentImage});
		return;
	}

	auto entity = _asset->GetEntity();

	auto model = entity->GetEditableModel();

	textureEntity->ShowUVMap = _ui.ShowUVMap->isChecked();

	const float scale = textureEntity->TextureScale;

	//Create an updated image of the UV map with current settings
	const auto uvMapImage = CreateUVMapImage(
		*model, textureIndex, GetMeshIndexForDrawing(_ui.Meshes),
		_ui.AntiAliasLines->isChecked(), scale, _ui.UVLineWidthSpinner->value());

	textureEntity->SetUVMeshImage({uvMapImage.width(), uvMapImage.height(), reinterpret_cast<const std::byte*>(uvMapImage.constBits())});
}

void TexturesPanel::OnImportTexture()
{
	auto model = _asset->GetEntity()->GetEditableModel();

	const int iTextureIndex = _ui.Textures->currentIndex();

	if (iTextureIndex == -1)
	{
		QMessageBox::information(this, "Message", "No texture selected");
		return;
	}

	const QString fileName = QFileDialog::getOpenFileName(this, {}, {}, qt::GetImagesFileFilter());

	if (fileName.isEmpty())
	{
		return;
	}

	ImportTextureFrom(fileName, *model, iTextureIndex);
}

void TexturesPanel::OnExportTexture()
{
	const int textureIndex = _ui.Textures->currentIndex();

	if (textureIndex == -1)
	{
		QMessageBox::information(this, "Message", "No texture selected");
		return;
	}

	auto model = _asset->GetEntity()->GetEditableModel();

	const auto& texture = *model->Textures[textureIndex];

	const QString fileName = QFileDialog::getSaveFileName(this, {}, texture.Name.c_str(), qt::GetSeparatedImagesFileFilter());

	if (fileName.isEmpty())
	{
		return;
	}

	auto textureImage = ConvertTextureToIndexed8Image(texture.Data);

	if (!textureImage.save(fileName))
	{
		QMessageBox::critical(this, "Error", QString{"Failed to save image \"%1\""}.arg(fileName));
	}
}

void TexturesPanel::OnExportUVMap()
{
	const int textureIndex = _ui.Textures->currentIndex();

	if (textureIndex == -1)
	{
		QMessageBox::information(this, "Message", "No texture selected");
		return;
	}

	auto entity = _asset->GetEntity();

	const auto model = entity->GetEditableModel();

	const auto& texture = *model->Textures[textureIndex];

	const auto textureData = texture.Data.Pixels.data();

	std::vector<QRgb> dataBuffer;

	auto textureImage{ConvertTextureToRGBImage(texture.Data, textureData, texture.Data.Palette, dataBuffer)};

	if (ExportUVMeshDialog dialog{*entity, textureIndex, GetMeshIndexForDrawing(_ui.Meshes), textureImage, this};
		QDialog::DialogCode::Accepted == dialog.exec())
	{
		//Redraw the final image with a transparent background
		const auto uvMapImage = dialog.GetUVImage();

		QImage resultImage{uvMapImage.width(), uvMapImage.height(), QImage::Format::Format_RGBA8888};

		//Set as transparent
		DrawUVImage(Qt::transparent, true, dialog.ShouldOverlayOnTexture(), textureImage, uvMapImage, resultImage);

		if (!dialog.ShouldAddAlphaChannel())
		{
			resultImage.convertTo(QImage::Format::Format_RGB888);
		}

		const QString fileName{dialog.GetFileName()};

		if (!resultImage.save(fileName))
		{
			QMessageBox::critical(this, "Error", QString{"Failed to save image \"%1\""}.arg(fileName));
		}
	}
}

void TexturesPanel::OnImportAllTextures()
{
	const auto path = QFileDialog::getExistingDirectory(this, "Select the directory to import all textures from");

	if (path.isEmpty())
	{
		return;
	}

	auto entity = _asset->GetEntity();

	auto model = entity->GetEditableModel();

	_asset->GetUndoStack()->beginMacro("Import all textures");

	//For each texture in the model, find if there is a file with the same name in the given directory
	//If so, try to replace the texture
	for (int i = 0; i < model->Textures.size(); ++i)
	{
		auto& texture = *model->Textures[i];

		const QFileInfo fileName{path, texture.Name.c_str()};

		if (fileName.exists())
		{
			ImportTextureFrom(fileName.absoluteFilePath(), *model, i);
		}
	}

	_asset->GetUndoStack()->endMacro();
}

void TexturesPanel::OnExportAllTextures()
{
	const auto path = QFileDialog::getExistingDirectory(this, "Select the directory to export all textures to");

	if (path.isEmpty())
	{
		return;
	}

	auto model = _asset->GetEntity()->GetEditableModel();

	QString errors;

	for (int i = 0; i < model->Textures.size(); ++i)
	{
		const auto& texture = *model->Textures[i];

		const QFileInfo fileName{path, texture.Name.c_str()};

		auto fullPath = fileName.absoluteFilePath();

		auto textureImage = ConvertTextureToIndexed8Image(texture.Data);

		if (!textureImage.save(fullPath))
		{
			errors += QString{"\"%1\"\n"}.arg(fullPath);
		}
	}

	if (!errors.isEmpty())
	{
		QMessageBox::warning(this, "One or more errors occurred", QString{"Failed to save images:\n%1"}.arg( errors));
	}
}

// TODO: check if this causes the colormap to update twice
void TexturesPanel::OnTopColorSliderChanged()
{
	_ui.TopColorSpinner->setValue(_ui.TopColorSlider->value());
	UpdateColormapValue();
}

void TexturesPanel::OnBottomColorSliderChanged()
{
	_ui.BottomColorSpinner->setValue(_ui.BottomColorSlider->value());
	UpdateColormapValue();
}

void TexturesPanel::OnTopColorSpinnerChanged()
{
	_ui.TopColorSlider->setValue(_ui.TopColorSpinner->value());
	UpdateColormapValue();
}

void TexturesPanel::OnBottomColorSpinnerChanged()
{
	_ui.BottomColorSlider->setValue(_ui.BottomColorSpinner->value());
	UpdateColormapValue();
}
}
