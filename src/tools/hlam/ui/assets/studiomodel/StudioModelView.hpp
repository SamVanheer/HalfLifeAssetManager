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
public:
	StudioModelView(QWidget* parent = nullptr);
	~StudioModelView();

	InfoBar* GetInfoBar() const { return _ui.InfoBar; }

	void AddWidget(QWidget* widget, const QString& label);

	void SetCurrentWidget(QWidget* widget);

private:
	Ui_StudioModelView _ui;
};
}
