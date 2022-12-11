#pragma once

#include <QString>
#include <QWidget>

#include "ui_LightingPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class LightingPanel final : public DockableWidget
{
public:
	LightingPanel(StudioModelAsset* asset);
	~LightingPanel();

private:
	void AddLight(const QString& name, QWidget* settingsPanel);

private:
	Ui_LightingPanel _ui;
	StudioModelAsset* const _asset;
};
}
