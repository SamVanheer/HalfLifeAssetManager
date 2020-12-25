#pragma once

#include <QString>
#include <QWidget>

#include "ui_StudioModelLightingPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelLightingPanel final : public QWidget
{
public:
	StudioModelLightingPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelLightingPanel();

private:
	void AddLight(const QString& name, QWidget* settingsPanel);

private:
	Ui_StudioModelLightingPanel _ui;
	StudioModelAsset* const _asset;
};
}
