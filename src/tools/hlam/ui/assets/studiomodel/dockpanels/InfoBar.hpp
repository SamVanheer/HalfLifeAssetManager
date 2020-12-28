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
	InfoBar(StudioModelAsset* asset, QWidget* parent = nullptr);

	~InfoBar();

public slots:
	void OnDraw();

private:
	Ui_InfoBar _ui;

	StudioModelAsset* const _asset;

	long long _lastFPSUpdate{0};
	unsigned int _currentFPS{0};

	unsigned int _oldDrawnPolygonsCount{0};
};
}
