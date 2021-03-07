#pragma once

#include <QWidget>

#include "ui_StudioModelModelDisplayPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelModelDisplayPanel final : public QWidget
{
public:
	StudioModelModelDisplayPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelModelDisplayPanel();

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
	void OnShowGroundChanged();
	void OnMirrorOnGroundChanged();
	void OnShowBackgroundChanged();

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

	void OnCenterModelOnWorldOrigin();

	void OnAlignOnGround();

	void OnEnableGroundTextureTilingChanged();
	void OnGroundTextureSizeChanged();

private:
	Ui_StudioModelModelDisplayPanel _ui;
	StudioModelAsset* const _asset;
};
}
