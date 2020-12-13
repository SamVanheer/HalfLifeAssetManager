#pragma once

#include <QWidget>

#include "ui_StudioModelBonesPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelBonesPanel final : public QWidget
{
public:
	StudioModelBonesPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelBonesPanel();

private:
	void UpdateQCString();

public slots:
	void OnDockPanelChanged(QWidget* current, QWidget* previous);

private slots:
	void OnBoneChanged(int index);
	void OnHightlightBoneChanged();

	void OnBoneNameChanged();

	void OnBoneParentChanged(int index);

	void OnBoneFlagsChanged();

	void OnBonePropertyChanged();

private:
	Ui_StudioModelBonesPanel _ui;
	StudioModelAsset* const _asset;

	bool _isActive{false};
};
}
