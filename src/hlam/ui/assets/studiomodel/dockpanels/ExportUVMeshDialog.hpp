#pragma once

#include <memory>

#include <QDialog>
#include <QImage>
#include <QString>

#include "ui_ExportUVMeshDialog.h"

class HLMVStudioModelEntity;

namespace studiomodel
{
class StudioModelAsset;

class ExportUVMeshDialog final : public QDialog
{
public:
	ExportUVMeshDialog(StudioModelAsset* asset,
		HLMVStudioModelEntity& entity, int textureIndex, int meshIndex, const QImage& texture, QWidget* parent);
	~ExportUVMeshDialog();

	QString GetFileName() const;

	double GetImageScale() const;

	double GetUVLineWidth() const;

	bool ShouldOverlayOnTexture() const;
	bool ShouldAntiAliasLines() const;
	bool ShouldAddAlphaChannel() const;

	QImage GetTextureImage() const { return _texture; }

	QImage GetUVImage() const { return _uv; }

	QImage GetPreviewImage() const { return _preview; }

protected:
	void resizeEvent(QResizeEvent* event) override;

private slots:
	void OnFileNameChanged();
	void OnBrowseFileName();

	void UpdatePreview();

private:
	Ui_ExportUVMeshDialog _ui;
	StudioModelAsset* const _asset;

	HLMVStudioModelEntity& _entity;
	const int _textureIndex;
	const int _meshIndex;

	const QImage _texture;
	QImage _uv;
	QImage _preview;
};
}
