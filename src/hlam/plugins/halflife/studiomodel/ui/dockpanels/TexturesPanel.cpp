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

#include "application/AssetManager.hpp"
#include "ui/StateSnapshot.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/settings/StudioModelSettings.hpp"

#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelEditWidget.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelTextureUtilities.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelValidators.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/ExportUVMeshDialog.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/TexturesPanel.hpp"

#include "ui/camera_operators/TextureCameraOperator.hpp"

namespace studiomodel
{
static constexpr double TextureViewScaleMinimum = 0.1;
static constexpr double TextureViewScaleMaximum = 20;
static constexpr double TextureViewScaleDefault = 1;
static constexpr double TextureViewScaleSingleStepValue = 0.1;
static constexpr double TextureViewScaleSliderRatio = 10.0;
static constexpr double UVLineWidthSliderRatio = 10.0;

const QString TextureExtension{QStringLiteral(".bmp")};

static int GetMeshIndexForDrawing(QComboBox* comboBox)
{
	int meshIndex = comboBox->currentIndex();

	if (comboBox->count() > 1 && meshIndex == (comboBox->count() - 1))
	{
		meshIndex = -1;
	}

	return meshIndex;
}

TexturesPanel::TexturesPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	const auto textureNameValidator = new UniqueTextureNameValidator(MaxTextureNameBytes - 1, _provider, this);

	_ui.TextureName->setValidator(textureNameValidator);

	_ui.ScaleTextureViewSpinner->setRange(TextureViewScaleMinimum, TextureViewScaleMaximum);
	_ui.ScaleTextureViewSpinner->setValue(TextureViewScaleDefault);
	_ui.ScaleTextureViewSpinner->setSingleStep(TextureViewScaleSingleStepValue);

	_ui.ScaleTextureViewSlider->setRange(
		0,
		static_cast<int>((_ui.ScaleTextureViewSpinner->maximum() - _ui.ScaleTextureViewSpinner->minimum()) * TextureViewScaleSliderRatio));

	_ui.ScaleTextureViewSlider->setValue(
		static_cast<int>((_ui.ScaleTextureViewSpinner->value() - _ui.ScaleTextureViewSpinner->minimum()) * TextureViewScaleSliderRatio));

	connect(_ui.Textures, qOverload<int>(&QComboBox::currentIndexChanged), textureNameValidator, &UniqueTextureNameValidator::SetCurrentIndex);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &TexturesPanel::OnAssetChanged);

	connect(_ui.Textures, qOverload<int>(&QComboBox::currentIndexChanged), this, &TexturesPanel::OnTextureChanged);
	connect(_ui.ScaleTextureViewSlider, &QSlider::valueChanged, this, &TexturesPanel::OnTextureViewScaleSliderChanged);
	connect(_ui.ScaleTextureViewSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TexturesPanel::OnTextureViewScaleSpinnerChanged);

	connect(_ui.TextureName, &QLineEdit::textChanged, this, &TexturesPanel::OnTextureNameChanged);
	connect(_ui.TextureName, &QLineEdit::inputRejected, this, &TexturesPanel::OnTextureNameRejected);

	connect(_ui.Chrome, &QCheckBox::stateChanged, this, &TexturesPanel::OnChromeChanged);
	connect(_ui.Additive, &QCheckBox::stateChanged, this, &TexturesPanel::OnAdditiveChanged);
	connect(_ui.Transparent, &QCheckBox::stateChanged, this, &TexturesPanel::OnTransparentChanged);
	connect(_ui.FlatShade, &QCheckBox::stateChanged, this, &TexturesPanel::OnFlatShadeChanged);
	connect(_ui.Fullbright, &QCheckBox::stateChanged, this, &TexturesPanel::OnFullbrightChanged);
	connect(_ui.Mipmaps, &QCheckBox::stateChanged, this, &TexturesPanel::OnMipmapsChanged);

	connect(_ui.ShowUVMap, &QCheckBox::stateChanged, this, &TexturesPanel::UpdateUVMapProperties);
	connect(_ui.OverlayUVMap, &QCheckBox::stateChanged, this, &TexturesPanel::OnOverlayUVMapChanged);
	connect(_ui.AntiAliasLines, &QCheckBox::stateChanged, this, &TexturesPanel::UpdateUVMapProperties);

	connect(_ui.Meshes, qOverload<int>(&QComboBox::currentIndexChanged), this,
		[this] { _asset->GetTextureEntity()->SetMeshIndex(GetMeshIndexForDrawing(_ui.Meshes)); });

	connect(_ui.ImportTexture, &QPushButton::clicked, this, &TexturesPanel::OnImportTexture);
	connect(_ui.ExportTexture, &QPushButton::clicked, this, &TexturesPanel::OnExportTexture);
	connect(_ui.ExportUVMap, &QPushButton::clicked, this, &TexturesPanel::OnExportUVMap);

	connect(_ui.ImportAllTextures, &QPushButton::clicked, this, &TexturesPanel::OnImportAllTextures);
	connect(_ui.ExportAllTextures, &QPushButton::clicked, this, &TexturesPanel::OnExportAllTextures);

	connect(_ui.TopColorSlider, &QSlider::valueChanged, this, &TexturesPanel::OnTopColorSliderChanged);
	connect(_ui.BottomColorSlider, &QSlider::valueChanged, this, &TexturesPanel::OnBottomColorSliderChanged);
	connect(_ui.TopColorSpinner, qOverload<int>(&QSpinBox::valueChanged), this, &TexturesPanel::OnTopColorSpinnerChanged);
	connect(_ui.BottomColorSpinner, qOverload<int>(&QSpinBox::valueChanged), this, &TexturesPanel::OnBottomColorSpinnerChanged);

	OnAssetChanged(_provider->GetDummyAsset());
}

TexturesPanel::~TexturesPanel() = default;

void TexturesPanel::OnVisibilityChanged(bool visible)
{
	if (_provider->GetStudioModelSettings()->ShouldActivateTextureViewWhenTexturesPanelOpened()
		&& _provider->GetEditWidget()->isVisible())
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
	ui.Mipmaps->setChecked((flags & STUDIO_NF_MIPMAPS) != 0);
}

void TexturesPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_asset)
	{
		_asset->disconnect(this);
		_asset->GetTextureCameraOperator()->disconnect(this);
	}

	_asset = asset;

	auto modelData = _asset->GetModelData();

	connect(_asset, &StudioModelAsset::SaveSnapshot, this, &TexturesPanel::OnSaveSnapshot);
	connect(_asset, &StudioModelAsset::LoadSnapshot, this, &TexturesPanel::OnLoadSnapshot);
	connect(_asset->GetTextureCameraOperator(), &TextureCameraOperator::ScaleChanged, this, &TexturesPanel::OnScaleChanged);

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
				SetTextureName(true);
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

	snapshot->SetValue("textures.mesh", _ui.Meshes->currentIndex());
}

void TexturesPanel::OnLoadSnapshot(StateSnapshot* snapshot)
{
	_updatingUI = true;

	auto model = _asset->GetEntity()->GetEditableModel();

	if (auto texture = snapshot->Value("textures.texture"); texture.isValid())
	{
		auto textureName = texture.toString().toStdString();

		if (auto it = std::find_if(model->Textures.begin(), model->Textures.end(), [&](const auto& texture)
			{
				return texture->Name == textureName;
			}); it != model->Textures.end())
		{
			const auto index = it - model->Textures.begin();

			_ui.Textures->setCurrentIndex(index);
		}
	}

	if (auto meshIndex = snapshot->Value("textures.mesh"); meshIndex.isValid())
	{
		_ui.Meshes->setCurrentIndex(meshIndex.toInt());
	}

	_ui.TopColorSlider->setValue(model->TopColor);
	_ui.BottomColorSlider->setValue(model->BottomColor);

	_updatingUI = false;
}

void TexturesPanel::OnScaleChanged(float adjust)
{
	_ui.ScaleTextureViewSpinner->setValue(_ui.ScaleTextureViewSpinner->value() + adjust);
}

void TexturesPanel::OnTextureChanged(int index)
{
	auto entity = _asset->GetEntity();

	const studiomdl::StudioTexture emptyTexture{};

	const auto& texture = index != -1 ? *entity->GetEditableModel()->Textures[index] : emptyTexture;

	SetTextureName(false);
	SetTextureFlagCheckBoxes(_ui, texture.Flags);

	const QSignalBlocker meshesBlocker{_ui.Meshes};

	_ui.Meshes->clear();
	_ui.Meshes->setEnabled(index != -1);

	const auto meshes = entity->GetEditableModel()->ComputeMeshList(index);

	for (decltype(meshes.size()) i = 0; i < meshes.size(); ++i)
	{
		_ui.Meshes->addItem(QString{"Mesh %1"}.arg(i + 1));
	}

	if (_ui.Meshes->count() > 1)
	{
		_ui.Meshes->addItem("All");
	}

	auto textureEntity = _asset->GetTextureEntity();

	//Reset texture position to be centered
	textureEntity->XOffset = textureEntity->YOffset = 0;
	textureEntity->SetTextureIndex(index, _ui.Meshes->currentIndex());

	UpdateUVMapProperties();
}

void TexturesPanel::OnTextureViewScaleSliderChanged(int value)
{
	const double newValue = (value / UVLineWidthSliderRatio) + _ui.ScaleTextureViewSpinner->minimum();

	{
		const QSignalBlocker blocker{_ui.ScaleTextureViewSpinner};
		_ui.ScaleTextureViewSpinner->setValue(newValue);
	}

	_asset->GetTextureEntity()->TextureScale = newValue;
}

void TexturesPanel::OnTextureViewScaleSpinnerChanged(double value)
{
	{
		const QSignalBlocker blocker{_ui.ScaleTextureViewSlider};
		_ui.ScaleTextureViewSlider->setValue(static_cast<int>((value - _ui.ScaleTextureViewSpinner->minimum()) * UVLineWidthSliderRatio));
	}

	_asset->GetTextureEntity()->TextureScale = value;
}

void TexturesPanel::SetTextureName(bool updateTextures)
{
	const QSignalBlocker blocker{_ui.TextureName};

	const int index = _ui.Textures->currentIndex();

	if (index == -1)
	{
		_ui.TextureName->setText(QString{});
		return;
	}

	QString name{QString::fromStdString(_asset->GetEditableStudioModel()->Textures[index]->Name)};

	name.chop(TextureExtension.size());

	//Avoid resetting the edit position
	if (_ui.TextureName->text() != name)
	{
		_ui.TextureName->setText(name);
	}

	if (updateTextures)
	{
		UpdateColormapValue();
	}
}

void TexturesPanel::OnTextureNameChanged()
{
	const auto& texture = *_asset->GetEntity()->GetEditableModel()->Textures[_ui.Textures->currentIndex()];

	auto newName = _ui.TextureName->text() + TextureExtension;

	_asset->AddUndoCommand(new ChangeTextureNameCommand(_asset, _ui.Textures->currentIndex(),
		QString::fromStdString(texture.Name), std::move(newName)));
}

void TexturesPanel::OnTextureNameRejected()
{
	QToolTip::showText(_ui.TextureName->mapToGlobal({0, -20}),
		"Texture names must be unique and cannot start/end with whitespace");
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

	const int flags = SetFlags(texture.Flags, STUDIO_NF_MIPMAPS, _ui.Mipmaps->isChecked());

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

	// For models with multiple skins we need to enforce the original texture dimensions
	// if any mesh that uses it uses alternate textures since the UV coordinates affect all textures.
	bool allowResizing = true;

	if (model.SkinFamilies.size() > 1)
	{
		for (std::size_t i = 0; i < model.SkinFamilies[0].size(); ++i)
		{
			std::size_t count = 0;

			for (std::size_t j = 0; j < model.SkinFamilies.size(); ++j)
			{
				if (textureIndex == model.SkinFamilies[j][i])
				{
					++count;
				}
			}

			if (count != 0 && count != model.SkinFamilies.size())
			{
				allowResizing = false;
				break;
			}
		}
	}

	auto& texture = *model.Textures[textureIndex];

	std::optional<QSize> requiredSize;

	if (!allowResizing)
	{
		requiredSize = QSize{ texture.Data.Width, texture.Data.Height };

		if (requiredSize != image.size())
		{
			if (QMessageBox::question(
				this, "Input required",
				"This texture is used in only some skins and must match the original dimensions to ensure UV coordinates are compatible with other skins.\nRescale image?",
				QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
			{
				return;
			}
		}
	}

	auto convertedTexture = ConvertImageToTexture(image, requiredSize);

	if (!convertedTexture)
	{
		QMessageBox::critical(this, "Error loading image", QString{"Palette for image \"%1\" does not exist."}.arg(fileName));
		return;
	}

	if (std::get<1>(convertedTexture.value()))
	{
		QMessageBox::warning(this, "Warning", "Image pixel count is not a multiple of 4 and will be upscaled");
	}

	if (std::get<2>(convertedTexture.value()))
	{
		QMessageBox::warning(this, "Warning",
			QString{"Image \"%1\" has the format \"%2\" and will be converted to an indexed 8 bit image. Loss of color depth may occur."}
			.arg(fileName)
			.arg(QMetaEnum::fromType<QImage::Format>().valueToKey(image.format())));
	}

	auto& textureData = std::get<0>(convertedTexture.value());

	auto scaledSTCoordinates = studiomdl::CalculateScaledSTCoordinatesData(
		model, textureIndex, texture.Data.Width, texture.Data.Height, textureData.Width, textureData.Height);

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
	if (_updatingUI)
	{
		return;
	}

	auto model = _asset->GetEditableStudioModel();

	model->TopColor = _ui.TopColorSlider->value();
	model->BottomColor = _ui.BottomColorSlider->value();

	const int colormapValue = model->TopColor & 0xFF | ((model->BottomColor & 0xFF) << 8);

	_ui.ColormapValue->setText(QString::number(colormapValue));

	auto graphicsContext = _asset->GetGraphicsContext();

	graphicsContext->Begin();
	model->UpdateTextures(*_asset->GetTextureLoader());
	graphicsContext->End();
}

void TexturesPanel::UpdateUVMapProperties()
{
	auto textureEntity = _asset->GetTextureEntity();

	textureEntity->ShowUVMap = _ui.ShowUVMap->isChecked();
	textureEntity->OverlayUVMap = _ui.OverlayUVMap->isChecked();
	textureEntity->AntiAliasLines = _ui.AntiAliasLines->isChecked();
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

	const QString fileName = QFileDialog::getOpenFileName(
		this, {}, _asset->GetApplication()->GetPath(TexturePathName), qt::GetImagesFileFilter());

	if (fileName.isEmpty())
	{
		return;
	}

	_asset->GetApplication()->SetPath(TexturePathName, fileName);

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

	const QString fileName = QFileDialog::getSaveFileName(
		this, {}, QString{"%1/%2"}
			.arg(_asset->GetApplication()->GetPath(TexturePathName))
			.arg(QString::fromStdString(texture.Name)),
		qt::GetSeparatedImagesFileFilter());

	if (fileName.isEmpty())
	{
		return;
	}

	auto textureImage = ConvertTextureToIndexed8Image(texture.Data);

	if (textureImage.save(fileName))
	{
		_asset->GetApplication()->SetPath(TexturePathName, fileName);
	}
	else
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

	if (ExportUVMeshDialog dialog{_asset, *entity, textureIndex, GetMeshIndexForDrawing(_ui.Meshes), textureImage, this};
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
	const auto path = QFileDialog::getExistingDirectory(
		this, "Select the directory to import all textures from",
		_asset->GetApplication()->GetPath(TexturePathName));

	if (path.isEmpty())
	{
		return;
	}

	_asset->GetApplication()->SetPath(TexturePathName, path);

	auto entity = _asset->GetEntity();

	auto model = entity->GetEditableModel();

	_asset->GetUndoStack()->beginMacro("Import all textures");

	//For each texture in the model, find if there is a file with the same name in the given directory
	//If so, try to replace the texture
	for (int i = 0; i < model->Textures.size(); ++i)
	{
		auto& texture = *model->Textures[i];

		const QFileInfo fileName{path, QString::fromStdString(texture.Name)};

		if (fileName.exists())
		{
			ImportTextureFrom(fileName.absoluteFilePath(), *model, i);
		}
	}

	_asset->GetUndoStack()->endMacro();
}

void TexturesPanel::OnExportAllTextures()
{
	const auto path = QFileDialog::getExistingDirectory(
		this, "Select the directory to export all textures to",
		_asset->GetApplication()->GetPath(TexturePathName));

	if (path.isEmpty())
	{
		return;
	}

	_asset->GetApplication()->SetPath(TexturePathName, path);

	auto model = _asset->GetEntity()->GetEditableModel();

	QString errors;

	for (int i = 0; i < model->Textures.size(); ++i)
	{
		const auto& texture = *model->Textures[i];

		const QFileInfo fileName{path, QString::fromStdString(texture.Name)};

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
}

void TexturesPanel::OnBottomColorSpinnerChanged()
{
	_ui.BottomColorSlider->setValue(_ui.BottomColorSpinner->value());
}
}
