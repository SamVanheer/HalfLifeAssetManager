#pragma once

#include <QMouseEvent>
#include <QWidget>

#include "ui_StudioModelTexturesPanel.h"

namespace studiomdl
{
class EditableStudioModel;
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
	StudioModelTexturesPanel(StudioModelAsset* asset);
	~StudioModelTexturesPanel();

private:
	void InitializeUI();

	void ImportTextureFrom(const QString& fileName, studiomdl::EditableStudioModel& model, int textureIndex);
	void RemapTexture(int index);
	void RemapTextures();
	void UpdateColormapValue();
	void UpdateUVMapTexture();

public slots:
	void AdjustScale(double amount);

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnScaleChanged(float adjust);

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

	void OnShowUVMapChanged();
	void OnOverlayUVMapChanged();
	void OnAntiAliasLinesChanged();

	void OnMeshChanged(int index);

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