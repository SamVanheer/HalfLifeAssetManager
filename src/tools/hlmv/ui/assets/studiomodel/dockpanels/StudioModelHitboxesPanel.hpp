#pragma once

#include <QWidget>

#include "ui_StudioModelHitboxesPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelHitboxesPanel final : public QWidget
{
public:
	StudioModelHitboxesPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelHitboxesPanel();

private:
	void UpdateQCString();

public slots:
	void OnDockPanelChanged(QWidget* current, QWidget* previous);

private slots:
	void OnHitboxChanged(int index);

	void OnHighlightHitboxChanged();

	void OnBoneChanged();
	void OnHitgroupChanged();

	void OnMinimumChanged();
	void OnMaximumChanged();

private:
	Ui_StudioModelHitboxesPanel _ui;
	StudioModelAsset* const _asset;

	bool _isActive{false};
};
}
