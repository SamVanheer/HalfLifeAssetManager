#pragma once

#include <QDialog>
#include <QImage>
#include <QString>

#include "ui_StudioModelExportUVMeshDialog.h"

class CHLMVStudioModelEntity;

namespace ui::assets::studiomodel
{
class StudioModelExportUVMeshDialog final : public QDialog
{
public:
	StudioModelExportUVMeshDialog(CHLMVStudioModelEntity* entity, int textureIndex, int meshIndex, const QImage& texture, QWidget* parent = nullptr);
	~StudioModelExportUVMeshDialog();

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
	Ui_StudioModelExportUVMeshDialog _ui;

	CHLMVStudioModelEntity* const _entity;
	const int _textureIndex;
	const int _meshIndex;

	const QImage _texture;
	QImage _uv;
	QImage _preview;

	QString _cachedFilter;
};
}
