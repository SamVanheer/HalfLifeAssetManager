#pragma once

#include <QWidget>

#include "ui_InfoBar.h"

#include "core/shared/Utility.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class InfoBar final : public QWidget
{
public:
	InfoBar(StudioModelContext* context, QWidget* parent = nullptr);

	~InfoBar();

public slots:
	void OnTick();

private:
	Ui_InfoBar _ui;

	StudioModelContext* const _context;

	long long _lastFPSUpdate{GetCurrentTick()};
	unsigned int _currentFPS{0};

	unsigned int _oldDrawnPolygonsCount{0};
};
}
