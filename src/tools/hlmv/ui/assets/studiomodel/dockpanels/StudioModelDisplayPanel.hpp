#pragma once

#include <QWidget>

#include "ui_StudioModelDisplayPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelDisplayPanel final : public QWidget
{
public:
	StudioModelDisplayPanel(StudioModelContext* context, QWidget* parent = nullptr);
	~StudioModelDisplayPanel();

private:
	Ui_StudioModelDisplayPanel _ui;
	StudioModelContext* const _context;
};
}
