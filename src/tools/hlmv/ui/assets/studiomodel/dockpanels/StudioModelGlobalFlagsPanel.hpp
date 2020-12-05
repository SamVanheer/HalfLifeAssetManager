#pragma once

#include <QWidget>

#include "ui_StudioModelGlobalFlagsPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelGlobalFlagsPanel final : public QWidget
{
public:
	StudioModelGlobalFlagsPanel(StudioModelContext* context, QWidget* parent = nullptr);
	~StudioModelGlobalFlagsPanel();

private slots:
	void OnFlagChanged(int state);

private:
	Ui_StudioModelGlobalFlagsPanel _ui;
	StudioModelContext* const _context;
};
}
