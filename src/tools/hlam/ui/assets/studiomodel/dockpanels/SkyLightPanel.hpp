#pragma once

#include <QWidget>

#include "ui_SkyLightPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class SkyLightPanel final : public QWidget
{
public:
	SkyLightPanel(StudioModelAsset* asset);
	~SkyLightPanel();

private slots:
	void OnAnglesChanged();

private:
	Ui_SkyLightPanel _ui;
	StudioModelAsset* const _asset;
};
}
