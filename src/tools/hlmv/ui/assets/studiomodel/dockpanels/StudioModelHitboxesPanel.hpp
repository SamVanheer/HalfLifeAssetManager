#pragma once

#include <QWidget>

#include "ui_StudioModelHitboxesPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelHitboxesPanel final : public QWidget
{
public:
	StudioModelHitboxesPanel(StudioModelContext* context, QWidget* parent = nullptr);
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
	StudioModelContext* const _context;

	bool _isActive{false};
};
}
