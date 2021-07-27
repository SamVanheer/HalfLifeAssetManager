#pragma once

#include <QWidget>

#include "ui_InfoBar.h"

#include "core/shared/Utility.hpp"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class InfoBar final : public QWidget
{
public:
	InfoBar(QWidget* parent = nullptr);

	~InfoBar();

	void SetAsset(StudioModelAsset* asset);

public slots:
	void OnDraw();
	void OnTick();

private:
	Ui_InfoBar _ui;

	StudioModelAsset* _asset = nullptr;

	long long _lastFPSUpdate{0};
	unsigned int _currentFPS{0};

	unsigned int _oldDrawnPolygonsCount{0};
};
}
