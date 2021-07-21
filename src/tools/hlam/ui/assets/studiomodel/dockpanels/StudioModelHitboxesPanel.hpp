#pragma once

#include <QWidget>

#include "ui_StudioModelHitboxesPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelHitboxesPanel final : public QWidget
{
public:
	StudioModelHitboxesPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelHitboxesPanel();

private:
	void InitializeUI();

	void UpdateQCString();

public slots:
	void OnDockPanelChanged(QWidget* current, QWidget* previous);

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnHitboxChanged(int index);

	void OnHighlightHitboxChanged();

	void OnBoneChanged();
	void OnHitgroupChanged();

	void OnBoundsChanged();
	void OnMaximumChanged();

private:
	Ui_StudioModelHitboxesPanel _ui;
	StudioModelAsset* const _asset;

	bool _isActive{false};
};
}
}
