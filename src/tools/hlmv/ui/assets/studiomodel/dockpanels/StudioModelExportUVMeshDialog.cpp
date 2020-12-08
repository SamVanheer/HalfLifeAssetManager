#include <QFileDialog>
#include <QImageWriter>
#include <QPainter>
#include <QPixmap>
#include <QStringList>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/dockpanels/StudioModelExportUVMeshDialog.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelTexturesPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelExportUVMeshDialog::StudioModelExportUVMeshDialog(
	CHLMVStudioModelEntity* entity, int textureIndex, int meshIndex, const QImage& texture, QWidget* parent)
	: QDialog(parent)
	, _entity(entity)
	, _textureIndex(textureIndex)
	, _meshIndex(meshIndex)
	, _texture(texture)
{
	_ui.setupUi(this);

	const auto studioTexture = entity->GetModel()->GetTextureHeader()->GetTexture(_textureIndex);

	connect(_ui.FileName, &QLineEdit::textChanged, this, &StudioModelExportUVMeshDialog::OnFileNameChanged);
	connect(_ui.BrowseFileName, &QPushButton::clicked, this, &StudioModelExportUVMeshDialog::OnBrowseFileName);

	connect(_ui.ImageSize, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelExportUVMeshDialog::UpdatePreview);
	connect(_ui.UVLineWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelExportUVMeshDialog::UpdatePreview);
	connect(_ui.OverlayOnTexture, &QCheckBox::stateChanged, this, &StudioModelExportUVMeshDialog::UpdatePreview);
	connect(_ui.AntiAliasLines, &QCheckBox::stateChanged, this, &StudioModelExportUVMeshDialog::UpdatePreview);
	connect(_ui.AddAlphaChannel, &QCheckBox::stateChanged, this, &StudioModelExportUVMeshDialog::UpdatePreview);

	_ui.TextureNameLabel->setText(studioTexture->name);

	_ui.OkButton->setEnabled(false);

	const auto formats = QImageWriter::supportedImageFormats();

	QStringList formatsStrings;

	formatsStrings.reserve(formats.size());

	for (const auto& format : formats)
	{
		formatsStrings.append(QString{"*.%1"}.arg(QString{format}));
	}

	_cachedFilter = QString{"Image Files (%1);;All Files (*.*)"}.arg(formatsStrings.join(' '));
}

StudioModelExportUVMeshDialog::~StudioModelExportUVMeshDialog() = default;

void StudioModelExportUVMeshDialog::resizeEvent(QResizeEvent* event)
{
	QDialog::resizeEvent(event);

	UpdatePreview();
}

QString StudioModelExportUVMeshDialog::GetFileName() const
{
	return _ui.FileName->text();
}

double StudioModelExportUVMeshDialog::GetImageScale() const
{
	return _ui.ImageSize->value() / 100.;
}

double StudioModelExportUVMeshDialog::GetUVLineWidth() const
{
	return _ui.UVLineWidth->value();
}

bool StudioModelExportUVMeshDialog::ShouldOverlayOnTexture() const
{
	return _ui.OverlayOnTexture->isChecked();
}

bool StudioModelExportUVMeshDialog::ShouldAntiAliasLines() const
{
	return _ui.AntiAliasLines->isChecked();
}

bool StudioModelExportUVMeshDialog::ShouldAddAlphaChannel() const
{
	return _ui.AddAlphaChannel->isChecked();
}

void StudioModelExportUVMeshDialog::OnFileNameChanged()
{
	_ui.OkButton->setEnabled(!_ui.FileName->text().isEmpty());
}

void StudioModelExportUVMeshDialog::OnBrowseFileName()
{
	const QString fileName = QFileDialog::getSaveFileName(this, "Select Image Filename", {}, _cachedFilter);

	if (!fileName.isEmpty())
	{
		_ui.FileName->setText(fileName);
	}
}

void StudioModelExportUVMeshDialog::UpdatePreview()
{
	_uv = StudioModelTexturesPanel::CreateUVMapImage(_entity, _textureIndex, _meshIndex,
		ShouldAntiAliasLines(),
		static_cast<float>(GetImageScale()),
		static_cast<qreal>(GetUVLineWidth()));

	_preview = QImage{_uv.width(), _uv.height(), QImage::Format::Format_RGBA8888};

	StudioModelTexturesPanel::DrawUVImage(Qt::black, ShouldOverlayOnTexture(), _texture, _uv, _preview);

	auto pixmap = QPixmap::fromImage(_preview);

	pixmap = pixmap.scaled(_ui.ImagePreview->width(), _ui.ImagePreview->height(), Qt::AspectRatioMode::KeepAspectRatio);

	_ui.ImagePreview->setPixmap(pixmap);
}
}
