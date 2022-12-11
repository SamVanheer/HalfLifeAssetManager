#pragma once

#include <QWidget>

#include "ui_HitboxesPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class HitboxesPanel final : public QWidget
{
public:
	HitboxesPanel(StudioModelAsset* asset);
	~HitboxesPanel();

private:
	void InitializeUI();

	void UpdateQCString();

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

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
