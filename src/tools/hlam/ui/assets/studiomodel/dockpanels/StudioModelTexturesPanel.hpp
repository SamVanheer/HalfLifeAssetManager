#pragma once

#include <QMouseEvent>
#include <QWidget>

#include "ui_StudioModelTexturesPanel.h"

class StudioModelEntity;

namespace studiomdl
{
class EditableStudioModel;
struct Texture;
}

namespace ui
{
class StateSnapshot;

namespace camera_operators
{
class CameraOperator;
}

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelTexturesPanel final : public QWidget
{
	Q_OBJECT

public:
	StudioModelTexturesPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelTexturesPanel();

	QImage GenerateTextureForDisplay();

private:
	void InitializeUI();

	void ImportTextureFrom(const QString& fileName, studiomdl::EditableStudioModel& model, int textureIndex);
	void RemapTexture(int index);
	void RemapTextures();
	void UpdateColormapValue();

signals:
	void CurrentTextureChanged();
	void TextureViewChanged();

public slots:
	void AdjustScale(double amount);

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnTextureChanged(int index);

	void OnTextureViewScaleSliderChanged(int value);
	void OnTextureViewScaleSpinnerChanged(double value);

	void OnUVLineWidthSliderChanged(int value);
	void OnUVLineWidthSpinnerChanged(double value);

	void OnTextureNameChanged();
	void OnTextureNameRejected();

	void OnChromeChanged();
	void OnAdditiveChanged();
	void OnTransparentChanged();
	void OnFlatShadeChanged();
	void OnFullbrightChanged();
	void OnMipmapsChanged();

	void OnImportTexture();
	void OnExportTexture();
	void OnExportUVMap();

	void OnImportAllTextures();
	void OnExportAllTextures();

	void OnTopColorSliderChanged();
	void OnBottomColorSliderChanged();
	void OnTopColorSpinnerChanged();
	void OnBottomColorSpinnerChanged();

	void OnTextureFiltersChanged();
	void OnPowerOf2TexturesChanged();

private:
	Ui_StudioModelTexturesPanel _ui;
	StudioModelAsset* const _asset;

	qreal _uvLineWidth{1};
};
}
}
