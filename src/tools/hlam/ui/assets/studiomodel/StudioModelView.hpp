#pragma once

#include <QString>
#include <QWidget>

#include "ui_StudioModelView.h"

namespace ui::assets::studiomodel
{
/**
*	@brief A collection of pages, an info bar and a tab bar to select the current page
*/
class StudioModelView final : public QWidget
{
	Q_OBJECT

public:
	StudioModelView(QWidget* parent);
	~StudioModelView();

	InfoBar* GetInfoBar() const { return _ui.InfoBar; }

	void AddWidget(QWidget* widget, const QString& label);

	void SetCurrentWidget(QWidget* widget);

signals:
	void PoseChanged(int index);

private slots:
	void OnPoseChanged(int index);

private:
	Ui_StudioModelView _ui;
};
}
