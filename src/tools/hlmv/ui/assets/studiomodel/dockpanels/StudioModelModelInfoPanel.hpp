#pragma once

#include <QWidget>

#include "ui_StudioModelModelInfoPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelModelInfoPanel final : public QWidget
{
public:
	StudioModelModelInfoPanel(StudioModelContext* context, QWidget* parent = nullptr);
	~StudioModelModelInfoPanel();

private:
	Ui_StudioModelModelInfoPanel _ui;
	StudioModelContext* const _context;
};
}
