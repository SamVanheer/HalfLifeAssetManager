#pragma once

#include <QSettings>
#include <QWidget>

#include "ui_OptionsPageGeneral.h"

namespace ui::options
{
class OptionsPageGeneral final : public QWidget
{
	Q_OBJECT

public:
	OptionsPageGeneral(QWidget* parent = nullptr);
	~OptionsPageGeneral();

public slots:
	void OnSaveChanges(QSettings& settings);

private:
	Ui_OptionsPageGeneral _ui;
};
}
