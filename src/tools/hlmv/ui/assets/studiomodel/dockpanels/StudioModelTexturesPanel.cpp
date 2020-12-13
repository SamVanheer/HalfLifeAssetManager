#include <utility>
#include <vector>

#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <QSignalBlocker>

#include <glm/vec2.hpp>

#include "entity/CHLMVStudioModelEntity.h"

#include "graphics/BMPFile.h"
#include "graphics/GraphicsUtils.h"
#include "graphics/IGraphicsContext.hpp"
#include "graphics/Palette.h"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelExportUVMeshDialog.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTexturesPanel.hpp"

namespace ui::assets::studiomodel
{
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

StudioModelTexturesPanel::StudioModelTexturesPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.Textures, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelTexturesPanel::OnTextureChanged);
	connect(_ui.ScaleTextureViewSlider, &QSlider::valueChanged, this, &StudioModelTexturesPanel::OnTextureViewScaleSliderChanged);
	connect(_ui.ScaleTextureViewSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelTexturesPanel::OnTextureViewScaleSpinnerChanged);
	connect(_ui.UVLineWidthSlider, &QSlider::valueChanged, this, &StudioModelTexturesPanel::OnUVLineWidthSliderChanged);
	connect(_ui.UVLineWidthSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelTexturesPanel::OnUVLineWidthSpinnerChanged);

	connect(_ui.Chrome, &QCheckBox::stateChanged, this, &StudioModelTexturesPanel::OnChromeChanged);
	connect(_ui.Additive, &QCheckBox::stateChanged, this, &StudioModelTexturesPanel::OnAdditiveChanged);
	connect(_ui.Transparent, &QCheckBox::stateChanged, this, &StudioModelTexturesPanel::OnTransparentChanged);
	connect(_ui.Fullbright, &QCheckBox::stateChanged, this, &StudioModelTexturesPanel::OnFullbrightChanged);

	connect(_ui.ShowUVMap, &QCheckBox::stateChanged, this, &StudioModelTexturesPanel::OnShowUVMapChanged);
	connect(_ui.OverlayUVMap, &QCheckBox::stateChanged, this, &StudioModelTexturesPanel::OnOverlayUVMapChanged);
	connect(_ui.AntiAliasLines, &QCheckBox::stateChanged, this, &StudioModelTexturesPanel::OnAntiAliasLinesChanged);

	connect(_ui.Meshes, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelTexturesPanel::OnMeshChanged);

	connect(_ui.ImportTexture, &QPushButton::clicked, this, &StudioModelTexturesPanel::OnImportTexture);
	connect(_ui.ExportTexture, &QPushButton::clicked, this, &StudioModelTexturesPanel::OnExportTexture);
	connect(_ui.ExportUVMap, &QPushButton::clicked, this, &StudioModelTexturesPanel::OnExportUVMap);

	connect(_ui.ImportAllTextures, &QPushButton::clicked, this, &StudioModelTexturesPanel::OnImportAllTextures);
	connect(_ui.ExportAllTextures, &QPushButton::clicked, this, &StudioModelTexturesPanel::OnExportAllTextures);

	connect(_ui.TopColorSlider, &QSlider::valueChanged, this, &StudioModelTexturesPanel::OnTopColorSliderChanged);
	connect(_ui.BottomColorSlider, &QSlider::valueChanged, this, &StudioModelTexturesPanel::OnBottomColorSliderChanged);
	connect(_ui.TopColorSpinner, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelTexturesPanel::OnTopColorSpinnerChanged);
	connect(_ui.BottomColorSpinner, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelTexturesPanel::OnBottomColorSpinnerChanged);

	_ui.ScaleTextureViewSlider->setRange(
		static_cast<int>(_ui.ScaleTextureViewSpinner->minimum() * TextureViewScaleSliderRatio),
		static_cast<int>(_ui.ScaleTextureViewSpinner->maximum() * TextureViewScaleSliderRatio));

	_ui.ScaleTextureViewSlider->setValue(static_cast<int>(_ui.ScaleTextureViewSpinner->value() * TextureViewScaleSliderRatio));

	_ui.UVLineWidthSlider->setRange(
		static_cast<int>(_ui.UVLineWidthSpinner->minimum() * UVLineWidthSliderRatio),
		static_cast<int>(_ui.UVLineWidthSpinner->maximum() * UVLineWidthSliderRatio));

	_ui.UVLineWidthSlider->setValue(static_cast<int>(_ui.UVLineWidthSpinner->value() * UVLineWidthSliderRatio));

	auto textureHeader = _asset->GetScene()->GetEntity()->GetModel()->GetTextureHeader();

	QStringList textures;

	textures.reserve(textureHeader->numtextures);

	for (int i = 0; i < textureHeader->numtextures; ++i)
	{
		auto texture = textureHeader->GetTexture(i);

		textures.append(QString{"%1 (%2 x %3)"}.arg(texture->name).arg(texture->width).arg(texture->height));
	}

	_ui.Textures->addItems(textures);
}

StudioModelTexturesPanel::~StudioModelTexturesPanel() = default;

void StudioModelTexturesPanel::OnMouseEvent(QMouseEvent* event)
{
	switch (event->type())
	{
	case QEvent::Type::MouseButtonPress:
	{
		//Only reset the position if a single button is down
		if (event->buttons() & (Qt::MouseButton::LeftButton | Qt::MouseButton::RightButton) && !(event->buttons() & (event->buttons() << 1)))
		{
			const auto position = event->pos();

			_dragPosition.x = position.x();
			_dragPosition.y = position.y();
		}
		break;
	}

	case QEvent::Type::MouseMove:
	{
		const glm::ivec2 position{event->pos().x(), event->pos().y()};

		const glm::ivec2 delta = position - _dragPosition;

		if (event->buttons() & Qt::MouseButton::LeftButton)
		{
			auto scene = _asset->GetScene();

			scene->TextureXOffset += delta.x;
			scene->TextureYOffset += delta.y;
		}
		else if (event->buttons() & Qt::MouseButton::RightButton)
		{
			const double zoomAdjust = delta.y / -20.0;

			_ui.ScaleTextureViewSpinner->setValue(_ui.ScaleTextureViewSpinner->value() + zoomAdjust);
		}

		_dragPosition = position;
		break;
	}
	}
}

QImage StudioModelTexturesPanel::CreateUVMapImage(
	CStudioModelEntity* entity, const int textureIndex, const int meshIndex, const bool antiAliasLines, float textureScale, qreal lineWidth)
{
	const auto model = entity->GetModel();

	const auto texture = model->GetTextureHeader()->GetTexture(textureIndex);

	//RGBA format because only the UV lines need to be drawn, with no background
	QImage image{static_cast<int>(std::ceil(texture->width * textureScale)), static_cast<int>(std::ceil(texture->height * textureScale)),
		QImage::Format::Format_RGBA8888};

	//Set as transparent
	image.fill(Qt::transparent);

	QPainter painter{&image};

	painter.setPen(QPen{Qt::white, lineWidth});
	painter.setRenderHint(QPainter::RenderHint::Antialiasing, antiAliasLines);

	auto fixCoords = [=](int x, int y)
	{
		return QPointF(x * textureScale, y * textureScale);
	};

	CStudioModelEntity::MeshList_t meshes = entity->ComputeMeshList(textureIndex);

	if (meshIndex != -1)
	{
		auto singleMesh = meshes[meshIndex];
		meshes.clear();
		meshes.emplace_back(singleMesh);
	}

	for (const auto mesh : meshes)
	{
		auto ptricmds = reinterpret_cast<const short*>(model->GetStudioHeader()->GetData() + mesh->triindex);

		for (int i; i = *(ptricmds++);)
		{
			if (i < 0)
			{
				i = -i;

				const auto firstVertex{fixCoords(ptricmds[2], ptricmds[3])};

				ptricmds += 4;
				--i;

				for (; i > 0; --i, ptricmds += 4)
				{
					painter.drawLine(firstVertex, fixCoords(ptricmds[2], ptricmds[3]));

					if (i > 1)
					{
						painter.drawLine(fixCoords(ptricmds[2], ptricmds[3]), fixCoords(ptricmds[6], ptricmds[7]));
					}
				}
			}
			else
			{
				auto firstVertex{fixCoords(ptricmds[2], ptricmds[3])};
				auto secondVertex{fixCoords(ptricmds[6], ptricmds[7])};

				painter.drawLine(firstVertex, secondVertex);

				ptricmds += 8;
				i -= 2;

				for (; i > 0; --i, ptricmds += 4)
				{
					painter.drawLine(secondVertex, fixCoords(ptricmds[2], ptricmds[3]));
					painter.drawLine(fixCoords(ptricmds[2], ptricmds[3]), firstVertex);

					firstVertex = secondVertex;
					secondVertex = fixCoords(ptricmds[2], ptricmds[3]);
				}
			}
		}
	}

	return image;
}

void StudioModelTexturesPanel::DrawUVImage(const QColor& backgroundColor, bool overlayOnTexture, const QImage& texture, const QImage& uvMap, QImage& target)
{
	target.fill(backgroundColor);

	QPainter painter{&target};

	const QRect drawRect{0, 0, target.width(), target.height()};

	if (overlayOnTexture)
	{
		painter.drawImage(drawRect, texture);
	}

	painter.drawImage(drawRect, uvMap);
}

void StudioModelTexturesPanel::OnCreateDeviceResources()
{
	//TODO: this shouldn't be done here
	RemapTextures();
}

void StudioModelTexturesPanel::OnDockPanelChanged(QWidget* current, QWidget* previous)
{
	const bool wasActive = _asset->GetScene()->ShowTexture;

	_asset->GetScene()->ShowTexture = this == current;

	if (_asset->GetScene()->ShowTexture && !wasActive)
	{
		_asset->PushInputSink(this);
	}
	else if (!_asset->GetScene()->ShowTexture && wasActive)
	{
		_asset->PopInputSink();
	}
}

void StudioModelTexturesPanel::OnTextureChanged(int index)
{
	auto scene = _asset->GetScene();

	//Reset texture position to be centered
	scene->TextureXOffset = scene->TextureYOffset = 0;

	_ui.Meshes->clear();

	if (index == -1)
	{
		_ui.Meshes->setEnabled(false);
		UpdateUVMapTexture();
		return;
	}

	auto entity = scene->GetEntity();

	auto textureHeader = entity->GetModel()->GetTextureHeader();

	auto texture = textureHeader->GetTexture(index);

	{
		const QSignalBlocker chrome{_ui.Chrome};
		const QSignalBlocker additive{_ui.Additive};
		const QSignalBlocker transparent{_ui.Transparent};
		const QSignalBlocker fullbright{_ui.Fullbright};

		_ui.Chrome->setChecked((texture->flags & STUDIO_NF_CHROME) != 0);
		_ui.Additive->setChecked((texture->flags & STUDIO_NF_ADDITIVE) != 0);
		_ui.Transparent->setChecked((texture->flags & STUDIO_NF_MASKED) != 0);
		_ui.Fullbright->setChecked((texture->flags & STUDIO_NF_FULLBRIGHT) != 0);
	}

	const CStudioModelEntity::MeshList_t meshes = entity->ComputeMeshList(index);

	_ui.Meshes->setEnabled(true);

	for (decltype(meshes.size()) i = 0; i < meshes.size(); ++i)
	{
		_ui.Meshes->addItem(QString{"Mesh %1"}.arg(i + 1));
	}

	if (_ui.Meshes->count() > 0)
	{
		if (_ui.Meshes->count() > 1)
		{
			_ui.Meshes->addItem("All");
		}
	}

	scene->TextureIndex = index;

	UpdateUVMapTexture();
}

void StudioModelTexturesPanel::OnTextureViewScaleSliderChanged(int value)
{
	const double newValue = value / UVLineWidthSliderRatio;

	{
		const QSignalBlocker blocker{_ui.ScaleTextureViewSpinner};
		_ui.ScaleTextureViewSpinner->setValue(newValue);
	}

	_asset->GetScene()->TextureScale = newValue;

	UpdateUVMapTexture();
}

void StudioModelTexturesPanel::OnTextureViewScaleSpinnerChanged(double value)
{
	{
		const QSignalBlocker blocker{_ui.ScaleTextureViewSlider};
		_ui.ScaleTextureViewSlider->setValue(static_cast<int>(value * UVLineWidthSliderRatio));
	}

	_asset->GetScene()->TextureScale = value;

	UpdateUVMapTexture();
}

void StudioModelTexturesPanel::OnUVLineWidthSliderChanged(int value)
{
	const double newValue = value / UVLineWidthSliderRatio;

	_uvLineWidth = static_cast<qreal>(newValue);

	{
		const QSignalBlocker blocker{_ui.UVLineWidthSpinner};
		_ui.UVLineWidthSpinner->setValue(newValue);
	}

	UpdateUVMapTexture();
}

void StudioModelTexturesPanel::OnUVLineWidthSpinnerChanged(double value)
{
	_uvLineWidth = static_cast<qreal>(value);

	{
		const QSignalBlocker blocker{_ui.UVLineWidthSlider};
		_ui.UVLineWidthSlider->setValue(static_cast<int>(value * UVLineWidthSliderRatio));
	}

	UpdateUVMapTexture();
}

void StudioModelTexturesPanel::OnChromeChanged()
{
	auto texture = _asset->GetScene()->GetEntity()->GetModel()->GetTextureHeader()->GetTexture(_ui.Textures->currentIndex());

	//Chrome disables alpha testing
	if (_ui.Chrome->isChecked())
	{
		_ui.Transparent->setChecked(false);
	}

	if (_ui.Chrome->isChecked())
	{
		texture->flags |= STUDIO_NF_CHROME;
	}
	else
	{
		texture->flags &= ~STUDIO_NF_CHROME;
	}

	//TODO:
	//m_pHLMV->GetState()->modelChanged = true;
}

void StudioModelTexturesPanel::OnAdditiveChanged()
{
	auto texture = _asset->GetScene()->GetEntity()->GetModel()->GetTextureHeader()->GetTexture(_ui.Textures->currentIndex());

	//Additive disables alpha testing
	if (_ui.Additive->isChecked())
	{
		_ui.Transparent->setChecked(false);
	}

	if (_ui.Additive->isChecked())
	{
		texture->flags |= STUDIO_NF_ADDITIVE;
	}
	else
	{
		texture->flags &= ~STUDIO_NF_ADDITIVE;
	}

	//TODO:
	//m_pHLMV->GetState()->modelChanged = true;
}

void StudioModelTexturesPanel::OnTransparentChanged()
{
	auto model = _asset->GetScene()->GetEntity()->GetModel();
	auto texture = model->GetTextureHeader()->GetTexture(_ui.Textures->currentIndex());

	//Alpha testing disables chrome and additive
	if (_ui.Transparent->isChecked())
	{
		_ui.Chrome->setChecked(false);
		_ui.Additive->setChecked(false);
	}

	if (_ui.Transparent->isChecked())
	{
		texture->flags |= STUDIO_NF_MASKED;
	}
	else
	{
		texture->flags &= ~STUDIO_NF_MASKED;
	}

	model->ReuploadTexture(texture);

	//TODO:
	//m_pHLMV->GetState()->modelChanged = true;
}

void StudioModelTexturesPanel::OnFullbrightChanged()
{
	auto texture = _asset->GetScene()->GetEntity()->GetModel()->GetTextureHeader()->GetTexture(_ui.Textures->currentIndex());

	if (_ui.Fullbright->isChecked())
	{
		texture->flags |= STUDIO_NF_FULLBRIGHT;
	}
	else
	{
		texture->flags &= ~STUDIO_NF_FULLBRIGHT;
	}

	//TODO:
	//m_pHLMV->GetState()->modelChanged = true;
}

void StudioModelTexturesPanel::OnShowUVMapChanged()
{
	UpdateUVMapTexture();
}

void StudioModelTexturesPanel::OnOverlayUVMapChanged()
{
	_asset->GetScene()->OverlayUVMap = _ui.OverlayUVMap->isChecked();
}

void StudioModelTexturesPanel::OnAntiAliasLinesChanged()
{
	UpdateUVMapTexture();
}

void StudioModelTexturesPanel::OnMeshChanged(int index)
{
	UpdateUVMapTexture();
}

void StudioModelTexturesPanel::ImportTextureFrom(const QString& fileName, studiomdl::CStudioModel* pStudioModel, studiohdr_t* pHdr, int textureIndex)
{
	//Must be BMP
	QImage image{fileName, "bmp"};

	if (image.isNull())
	{
		QMessageBox::critical(this, "Error loading image", QString{"Failed to load image \"%1\""}.arg(fileName));
		return;
	}

	if (image.format() != QImage::Format::Format_Indexed8)
	{
		QMessageBox::critical(this, "Error loading image", QString{"Image \"%1\" has unsupported format"}.arg(fileName));
		return;
	}

	const QVector<QRgb> palette = image.colorTable();

	if (palette.isEmpty())
	{
		QMessageBox::critical(this, "Error loading image", QString{"Palette for image \"%1\" does not exist"}.arg(fileName));
		return;
	}

	mstudiotexture_t& texture = ((mstudiotexture_t*)((byte*)pHdr + pHdr->textureindex))[textureIndex];

	if (texture.width != image.width() || texture.height != image.height())
	{
		QMessageBox::critical(this, "Error loading image",
			QString{"Image \"%1\" does not have matching dimensions to the current texture (src: %2 x %3, dest: %4 x %5)"}
				.arg(fileName)
				.arg(image.width()).arg(image.height())
				.arg(texture.width).arg(texture.height));
		return;
	}

	//Convert to 8 bit palette based image
	std::unique_ptr<byte[]> texData = std::make_unique<byte[]>(image.width() * image.height());

	{
		byte* pDest = texData.get();

		for (int y = 0; y < image.height(); ++y)
		{
			for (int x = 0; x < image.width(); ++x, ++pDest)
			{
				*pDest = image.pixelIndex(x, y);
			}
		}
	}

	byte convPal[PALETTE_SIZE];

	memset(convPal, 0, sizeof(convPal));

	int paletteIndex;

	for (paletteIndex = 0; paletteIndex < palette.size(); ++paletteIndex)
	{
		const auto rgb = palette[paletteIndex];

		convPal[paletteIndex * PALETTE_CHANNELS] = qRed(rgb);
		convPal[paletteIndex * PALETTE_CHANNELS + 1] = qGreen(rgb);
		convPal[paletteIndex * PALETTE_CHANNELS + 2] = qBlue(rgb);
	}

	//Fill remaining entries with black
	for (; paletteIndex < PALETTE_ENTRIES; ++paletteIndex)
	{
		convPal[paletteIndex * PALETTE_CHANNELS] = 0;
		convPal[paletteIndex * PALETTE_CHANNELS + 1] = 0;
		convPal[paletteIndex * PALETTE_CHANNELS + 2] = 0;
	}

	//Copy over the new image data to the texture.
	memcpy((byte*)pHdr + texture.index, texData.get(), image.width() * image.height());
	memcpy((byte*)pHdr + texture.index + image.width() * image.height(), convPal, PALETTE_SIZE);

	pStudioModel->ReplaceTexture(&texture, texData.get(), convPal, pStudioModel->GetTextureId(textureIndex));

	//TODO:
	//m_pHLMV->GetState()->modelChanged = true;
}

void StudioModelTexturesPanel::RemapTexture(int index)
{
	auto entity = _asset->GetScene()->GetEntity();

	auto textureHeader = entity->GetModel()->GetTextureHeader();

	const auto texture = textureHeader->GetTexture(index);

	const auto textureId = entity->GetModel()->GetTextureId(index);

	int low, mid, high;

	if (graphics::TryGetRemapColors(texture->name, low, mid, high))
	{
		byte palette[PALETTE_SIZE];

		memcpy(palette, reinterpret_cast<byte*>(textureHeader) + texture->index + texture->width * texture->height, PALETTE_SIZE);

		graphics::PaletteHueReplace(palette, _ui.TopColorSlider->value(), low, mid);

		if (high)
		{
			graphics::PaletteHueReplace(palette, _ui.BottomColorSlider->value(), mid + 1, high);
		}

		entity->GetModel()->ReplaceTexture(texture, reinterpret_cast<byte*>(textureHeader) + texture->index, palette, textureId);
	}
}

void StudioModelTexturesPanel::RemapTextures()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto textureHeader = entity->GetModel()->GetTextureHeader();

	for (int i = 0; i < textureHeader->numtextures; ++i)
	{
		RemapTexture(i);
	}
}

void StudioModelTexturesPanel::UpdateColormapValue()
{
	const auto topColor = _ui.TopColorSlider->value();
	const auto bottomColor = _ui.BottomColorSlider->value();

	const int colormapValue = topColor & 0xFF | ((bottomColor & 0xFF) << 8);

	_ui.ColormapValue->setText(QString::number(colormapValue));
}

void StudioModelTexturesPanel::UpdateUVMapTexture()
{
	if (!_ui.ShowUVMap->isChecked())
	{
		return;
	}

	auto scene = _asset->GetScene();

	const int textureIndex = _ui.Textures->currentIndex();

	if (textureIndex == -1)
	{
		//Make sure nothing is drawn when no texture is selected
		const byte transparentImage[4] = {0, 0, 0, 0};

		glBindTexture(GL_TEXTURE_2D, scene->UVMeshTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			1, 1,
			0, GL_RGBA, GL_UNSIGNED_BYTE, transparentImage);
		return;
	}

	auto entity = scene->GetEntity();

	auto model = entity->GetModel();

	auto texture = model->GetTextureHeader()->GetTexture(textureIndex);

	scene->ShowUVMap = _ui.ShowUVMap->isChecked();

	const float scale = scene->TextureScale;

	//Create an updated image of the UV map with current settings
	const auto uvMapImage = CreateUVMapImage(entity, textureIndex, GetMeshIndexForDrawing(_ui.Meshes), _ui.AntiAliasLines->isChecked(), scale, _uvLineWidth);

	scene->GetGraphicsContext()->Begin();

	glBindTexture(GL_TEXTURE_2D, scene->UVMeshTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		static_cast<int>(std::ceil(texture->width * scale)), static_cast<int>(std::ceil(texture->height * scale)),
		0, GL_RGBA, GL_UNSIGNED_BYTE, uvMapImage.constBits());

	//Nearest filtering causes gaps in lines, linear does not
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Prevent the texture from wrapping and spilling over on the other side
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	scene->GetGraphicsContext()->End();
}

void StudioModelTexturesPanel::OnImportTexture()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto pStudioModel = entity->GetModel();

	const int iTextureIndex = _ui.Textures->currentIndex();

	if (iTextureIndex == -1)
	{
		QMessageBox::information(this, "Message", "No texture selected");
		return;
	}

	const QString fileName = QFileDialog::getOpenFileName(this, {}, {}, "Windows Bitmap (*.bmp)");

	if (fileName.isEmpty())
	{
		return;
	}

	ImportTextureFrom(fileName, pStudioModel, pStudioModel->GetTextureHeader(), iTextureIndex);

	RemapTexture(iTextureIndex);
}

void StudioModelTexturesPanel::OnExportTexture()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto pStudioModel = entity->GetModel();

	const int iTextureIndex = _ui.Textures->currentIndex();

	if (iTextureIndex == -1)
	{
		QMessageBox::information(this, "Message", "No texture selected");
		return;
	}

	studiohdr_t* const pHdr = pStudioModel->GetTextureHeader();

	mstudiotexture_t& texture = ((mstudiotexture_t*)((byte*)pHdr + pHdr->textureindex))[iTextureIndex];

	const QString fileName = QFileDialog::getSaveFileName(this, {}, {}, "Windows Bitmap (*.bmp)");

	if (fileName.isEmpty())
	{
		return;
	}

	if (!graphics::bmpfile::SaveBMPFile(fileName.toUtf8().constData(), texture.width, texture.height,
		(uint8_t*)pHdr + texture.index, (uint8_t*)pHdr + texture.index + texture.width * texture.height))
	{
		QMessageBox::critical(this, "Error", QString{"Failed to save image \"%1\""}.arg(fileName));
	}
}

void StudioModelTexturesPanel::OnExportUVMap()
{
	auto entity = _asset->GetScene()->GetEntity();

	const int textureIndex = _ui.Textures->currentIndex();

	if (textureIndex == -1)
	{
		QMessageBox::information(this, "Message", "No texture selected");
		return;
	}

	const auto header = entity->GetModel()->GetTextureHeader();

	const auto texture = header->GetTexture(textureIndex);

	const auto textureData = header->GetData() + texture->index;
	const auto texturePalette = header->GetData() + texture->index + (texture->width * texture->height);

	std::vector<QRgb> imageData;

	imageData.resize(texture->width * texture->height);

	for (int y = 0; y < texture->height; ++y)
	{
		for (int x = 0; x < texture->width; ++x)
		{
			const auto color = texturePalette + (textureData[(texture->width * y) + x] * 3);

			imageData[(texture->width * y) + x] = qRgb(color[0], color[1], color[2]);
		}
	}

	QImage textureImage{reinterpret_cast<const uchar*>(imageData.data()), texture->width, texture->height, QImage::Format::Format_RGB32};

	if (StudioModelExportUVMeshDialog dialog{entity, textureIndex, GetMeshIndexForDrawing(_ui.Meshes), textureImage, this};
		QDialog::DialogCode::Accepted == dialog.exec())
	{
		//Redraw the final image with a transparent background
		const auto uvMapImage = dialog.GetUVImage();

		QImage resultImage{uvMapImage.width(), uvMapImage.height(), QImage::Format::Format_RGBA8888};

		//Set as transparent
		StudioModelTexturesPanel::DrawUVImage(Qt::transparent, dialog.ShouldOverlayOnTexture(), textureImage, dialog.GetUVImage(), resultImage);

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

void StudioModelTexturesPanel::OnImportAllTextures()
{
	auto entity = _asset->GetScene()->GetEntity();

	const auto path = QFileDialog::getExistingDirectory(this, "Select the directory to import all textures from");

	if (path.isEmpty())
	{
		return;
	}

	auto pStudioModel = entity->GetModel();
	studiohdr_t* const pHdr = pStudioModel->GetTextureHeader();

	//For each texture in the model, find if there is a file with the same name in the given directory
	//If so, try to replace the texture
	for (int i = 0; i < pHdr->numtextures; ++i)
	{
		mstudiotexture_t& texture = ((mstudiotexture_t*)((byte*)pHdr + pHdr->textureindex))[i];

		const QFileInfo fileName{path, texture.name};

		if (fileName.exists())
		{
			ImportTextureFrom(fileName.absoluteFilePath(), pStudioModel, pHdr, i);
		}
	}

	RemapTextures();
}

void StudioModelTexturesPanel::OnExportAllTextures()
{
	auto entity = _asset->GetScene()->GetEntity();

	auto pStudioModel = entity->GetModel();

	const auto path = QFileDialog::getExistingDirectory(this, "Select the directory to export all textures to");

	if (path.isEmpty())
	{
		return;
	}

	studiohdr_t* const pHdr = pStudioModel->GetTextureHeader();

	QString errors;

	for (int i = 0; i < pHdr->numtextures; ++i)
	{
		const auto& texture = ((mstudiotexture_t*)((byte*)pHdr + pHdr->textureindex))[i];

		const QFileInfo fileName{path, texture.name};

		auto fullPath = fileName.absoluteFilePath();

		if (!graphics::bmpfile::SaveBMPFile(fullPath.toUtf8().constData(), texture.width, texture.height,
			(uint8_t*)pHdr + texture.index, (uint8_t*)pHdr + texture.index + texture.width * texture.height))
		{
			errors += QString{"\"%1\"\n"}.arg(fullPath);
		}
	}

	if (!errors.isEmpty())
	{
		QMessageBox::warning(this, "One or more errors occurred", QString{"Failed to save images:\n%1"}.arg( errors));
	}
}

void StudioModelTexturesPanel::OnTopColorSliderChanged()
{
	_ui.TopColorSpinner->setValue(_ui.TopColorSlider->value());

	UpdateColormapValue();
	RemapTextures();
}

void StudioModelTexturesPanel::OnBottomColorSliderChanged()
{
	_ui.BottomColorSpinner->setValue(_ui.BottomColorSlider->value());

	UpdateColormapValue();
	RemapTextures();
}

void StudioModelTexturesPanel::OnTopColorSpinnerChanged()
{
	_ui.TopColorSlider->setValue(_ui.TopColorSpinner->value());

	UpdateColormapValue();
	RemapTextures();
}

void StudioModelTexturesPanel::OnBottomColorSpinnerChanged()
{
	_ui.BottomColorSlider->setValue(_ui.BottomColorSpinner->value());

	UpdateColormapValue();
	RemapTextures();
}
}
