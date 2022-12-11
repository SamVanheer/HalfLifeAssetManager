#pragma once

#include <QString>
#include <QWidget>

#include "ui_LightingPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class LightingPanel final : public QWidget
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
