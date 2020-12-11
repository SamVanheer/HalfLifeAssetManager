#pragma once

#include <QWidget>

#include "ui_StudioModelModelDisplayPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelModelDisplayPanel final : public QWidget
{
public:
	StudioModelModelDisplayPanel(StudioModelContext* context, QWidget* parent = nullptr);
	~StudioModelModelDisplayPanel();

private:
	void OnRenderModeChanged(int index);
	void OnOpacityChanged(int value);

	void OnShowHitboxesChanged();
	void OnShowBonesChanged();
	void OnShowAttachmentsChanged();
	void OnShowEyePositionChanged();

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

	void OnFieldOfViewChanged(double value);

	void OnResetFieldOfView();

	void OnFirstPersonFieldOfViewChanged(double value);

	void OnResetFirstPersonFieldOfView();

	void OnCenterModelOnWorldOrigin();

	void OnAlignOnGround();

private:
	Ui_StudioModelModelDisplayPanel _ui;
	StudioModelContext* const _context;
};
}
