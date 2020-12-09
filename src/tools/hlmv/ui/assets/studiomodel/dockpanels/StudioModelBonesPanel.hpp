#pragma once

#include <QWidget>

#include "ui_StudioModelBonesPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelBonesPanel final : public QWidget
{
public:
	StudioModelBonesPanel(StudioModelContext* context, QWidget* parent = nullptr);
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
	StudioModelContext* const _context;

	bool _isActive{false};
};
}
