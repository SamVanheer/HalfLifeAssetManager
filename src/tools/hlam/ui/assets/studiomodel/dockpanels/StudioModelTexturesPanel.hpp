#pragma once

#include <QMouseEvent>
#include <QWidget>

#include <glm/vec2.hpp>

#include "ui_StudioModelTexturesPanel.h"

#include "ui/IInputSink.hpp"

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

class StudioModelTexturesPanel final : public QWidget, public IInputSink
{
public:
	StudioModelTexturesPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelTexturesPanel();

	void OnMouseEvent(QMouseEvent* event) override;

	void OnWheelEvent(QWheelEvent*) {}

	static QImage CreateUVMapImage(
		StudioModelEntity* entity, const int textureIndex, const int meshIndex, const bool antiAliasLines, float textureScale, qreal lineWidth);

	static void DrawUVImage(const QColor& backgroundColor, bool overlayOnTexture, const QImage& texture, const QImage& uvMap, QImage& target);

private:
	void InitializeUI();

	void ImportTextureFrom(const QString& fileName, studiomdl::EditableStudioModel& model, int textureIndex);
	bool ExportTextureTo(const QString& fileName, const studiomdl::EditableStudioModel& model, const studiomdl::Texture& texture);
	void RemapTexture(int index);
	void RemapTextures();
	void UpdateColormapValue();

	void UpdateUVMapTexture();

public slots:
	void OnCreateDeviceResources();

	void OnDockPanelChanged(QWidget* current, QWidget* previous);

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

	Qt::MouseButtons _trackedMouseButtons;
	glm::ivec2 _dragPosition{0};

	qreal _uvLineWidth{1};
};
}
}
