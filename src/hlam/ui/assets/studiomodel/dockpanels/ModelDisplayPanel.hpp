#pragma once

#include <QWidget>

#include "ui_ModelDisplayPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class ModelDisplayPanel final : public QWidget
{
public:
	ModelDisplayPanel(StudioModelAsset* asset);
	~ModelDisplayPanel();

private:
	void OnRenderModeChanged(int index);
	void OnOpacityChanged(int value);

	void OnShowHitboxesChanged();
	void OnShowBonesChanged();
	void OnShowAttachmentsChanged();
	void OnShowEyePositionChanged();

	void OnShowBBoxChanged();
	void OnShowCBoxChanged();

	void OnEnableBackfaceCullingChanged();

	void OnWireframeOverlayChanged();
	void OnDrawShadowsChanged();
	void OnFixShadowZFightingChanged();
	void OnShowAxesChanged();

	void OnShowNormalsChanged();
	void OnShowCrosshairChanged();
	void OnShowGuidelinesChanged();
	void OnShowPlayerHitboxChanged();

	void OnMirrorXAxisChanged();
	void OnMirrorYAxisChanged();
	void OnMirrorZAxisChanged();

private:
	Ui_ModelDisplayPanel _ui;
	StudioModelAsset* const _asset;
};
}
