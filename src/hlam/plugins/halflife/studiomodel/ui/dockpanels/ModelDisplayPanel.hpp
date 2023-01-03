#pragma once

#include "ui_ModelDisplayPanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;

class ModelDisplayPanel final : public DockableWidget
{
public:
	explicit ModelDisplayPanel(StudioModelAssetProvider* provider);
	~ModelDisplayPanel();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

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
	void OnShowPlayerHitboxChanged();

	void OnMirrorXAxisChanged();
	void OnMirrorYAxisChanged();
	void OnMirrorZAxisChanged();

private:
	Ui_ModelDisplayPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
};
}
