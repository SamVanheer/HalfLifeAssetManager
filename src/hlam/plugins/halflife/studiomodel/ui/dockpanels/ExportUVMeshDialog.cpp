#include <QFileDialog>
#include <QImageWriter>
#include <QPainter>
#include <QPixmap>
#include <QStringList>

#include "entity/HLMVStudioModelEntity.hpp"

#include "qt/QtUtilities.hpp"

#include "ui/EditorContext.hpp"
#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelTextureUtilities.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/ExportUVMeshDialog.hpp"
#include "plugins/halflife/studiomodel/ui/dockpanels/TexturesPanel.hpp"

namespace studiomodel
{
ExportUVMeshDialog::ExportUVMeshDialog(StudioModelAsset* asset,
	HLMVStudioModelEntity& entity, int textureIndex, int meshIndex, const QImage& texture, QWidget* parent)
	: QDialog(parent)
	, _asset(asset)
	, _entity(entity)
	, _textureIndex(textureIndex)
	, _meshIndex(meshIndex)
	, _texture(texture)
{
	_ui.setupUi(this);

	const auto& studioTexture = *entity.GetEditableModel()->Textures[_textureIndex];

	connect(_ui.FileName, &QLineEdit::textChanged, this, &ExportUVMeshDialog::OnFileNameChanged);
	connect(_ui.BrowseFileName, &QPushButton::clicked, this, &ExportUVMeshDialog::OnBrowseFileName);

	connect(_ui.ImageSize, qOverload<int>(&QSpinBox::valueChanged), this, &ExportUVMeshDialog::UpdatePreview);
	connect(_ui.UVLineWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ExportUVMeshDialog::UpdatePreview);
	connect(_ui.OverlayOnTexture, &QCheckBox::stateChanged, this, &ExportUVMeshDialog::UpdatePreview);
	connect(_ui.AntiAliasLines, &QCheckBox::stateChanged, this, &ExportUVMeshDialog::UpdatePreview);
	connect(_ui.AddAlphaChannel, &QCheckBox::stateChanged, this, &ExportUVMeshDialog::UpdatePreview);

	_ui.TextureNameLabel->setText(studioTexture.Name.c_str());

	_ui.OkButton->setEnabled(false);
}

ExportUVMeshDialog::~ExportUVMeshDialog() = default;

void ExportUVMeshDialog::resizeEvent(QResizeEvent* event)
{
	QDialog::resizeEvent(event);

	UpdatePreview();
}

QString ExportUVMeshDialog::GetFileName() const
{
	return _ui.FileName->text();
}

double ExportUVMeshDialog::GetImageScale() const
{
	return _ui.ImageSize->value() / 100.;
}

double ExportUVMeshDialog::GetUVLineWidth() const
{
	return _ui.UVLineWidth->value();
}

bool ExportUVMeshDialog::ShouldOverlayOnTexture() const
{
	return _ui.OverlayOnTexture->isChecked();
}

bool ExportUVMeshDialog::ShouldAntiAliasLines() const
{
	return _ui.AntiAliasLines->isChecked();
}

bool ExportUVMeshDialog::ShouldAddAlphaChannel() const
{
	return _ui.AddAlphaChannel->isChecked();
}

void ExportUVMeshDialog::OnFileNameChanged()
{
	_ui.OkButton->setEnabled(!_ui.FileName->text().isEmpty());
}

void ExportUVMeshDialog::OnBrowseFileName()
{
	const QString fileName = QFileDialog::getSaveFileName(
		this, "Select Image Filename", _asset->GetEditorContext()->GetPath(TexturePathName),
		qt::GetSeparatedImagesFileFilter());

	if (!fileName.isEmpty())
	{
		_asset->GetEditorContext()->SetPath(TexturePathName, fileName);
		_ui.FileName->setText(fileName);
	}
}

void ExportUVMeshDialog::UpdatePreview()
{
	_uv = CreateUVMapImage(*_entity.GetEditableModel(), _textureIndex, _meshIndex,
		ShouldAntiAliasLines(),
		static_cast<float>(GetImageScale()),
		static_cast<qreal>(GetUVLineWidth()));

	_preview = QImage{_uv.width(), _uv.height(), QImage::Format::Format_RGBA8888};

	DrawUVImage(Qt::black, true, ShouldOverlayOnTexture(), _texture, _uv, _preview);

	auto pixmap = QPixmap::fromImage(_preview);

	pixmap = pixmap.scaled(_ui.ImagePreview->width(), _ui.ImagePreview->height(), Qt::AspectRatioMode::KeepAspectRatio);

	_ui.ImagePreview->setPixmap(pixmap);
}
}
