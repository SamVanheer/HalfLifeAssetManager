#pragma once

#include "ui_HitboxesPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class HitboxesPanel final : public DockableWidget
{
public:
	HitboxesPanel(StudioModelAsset* asset);
	~HitboxesPanel();

private:
	void UpdateQCString();

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnHitboxChanged(int index);

	void OnHighlightHitboxChanged();

	void OnBoneChanged();
	void OnHitgroupChanged();

	void OnBoundsChanged();

private:
	Ui_HitboxesPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
