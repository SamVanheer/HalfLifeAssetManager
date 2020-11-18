#pragma once

#include <QWidget>

#include "ui_StudioModelDisplayPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelDisplayPanel final : public QWidget
{
public:
	StudioModelDisplayPanel(QWidget* parent = nullptr);
	~StudioModelDisplayPanel();

private:
	Ui_StudioModelDisplayPanel _ui;
};
}
