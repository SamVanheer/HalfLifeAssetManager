#pragma once

#include <QWidget>

#include <glm/vec2.hpp>

#include "ui_StudioModelTexturesPanel.h"

#include "ui/IInputSink.hpp"

class CStudioModelEntity;

namespace ui
{
namespace camera_operators
{
class CameraOperator;
}

namespace assets::studiomodel
{
class StudioModelContext;

class StudioModelTexturesPanel final : public QWidget, public IInputSink
{
public:
	StudioModelTexturesPanel(StudioModelContext* context, QWidget* parent = nullptr);
	~StudioModelTexturesPanel();

	void OnMouseEvent(QMouseEvent* event) override;

	static QImage CreateUVMapImage(
		CStudioModelEntity* entity, const int textureIndex, const int meshIndex, const bool antiAliasLines, float textureScale, qreal lineWidth);

	static void DrawUVImage(const QColor& backgroundColor, bool overlayOnTexture, const QImage& texture, const QImage& uvMap, QImage& target);

private:
	void ImportTextureFrom(const QString& fileName, studiomdl::CStudioModel* pStudioModel, studiohdr_t* pHdr, int textureIndex);
	void RemapTexture(int index);
	void RemapTextures();
	void UpdateColormapValue();

	void UpdateUVMapTexture();

public slots:
	void OnCreateDeviceResources();

	void OnDockPanelChanged(QWidget* current, QWidget* previous);

private slots:
	void OnTextureChanged(int index);

	void OnTextureViewScaleSliderChanged(int value);
	void OnTextureViewScaleSpinnerChanged(double value);

	void OnUVLineWidthSliderChanged(int value);
	void OnUVLineWidthSpinnerChanged(double value);

	void OnChromeChanged();
	void OnAdditiveChanged();
	void OnTransparentChanged();
	void OnFullbrightChanged();

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

private:
	Ui_StudioModelTexturesPanel _ui;
	StudioModelContext* const _context;

	glm::ivec2 _dragPosition{0};

	qreal _uvLineWidth{1};
};
}
}
