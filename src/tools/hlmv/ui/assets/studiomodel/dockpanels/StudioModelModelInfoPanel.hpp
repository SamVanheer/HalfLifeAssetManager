#pragma once

#include <QWidget>

#include "ui_StudioModelModelInfoPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelModelInfoPanel final : public QWidget
{
public:
	StudioModelModelInfoPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelModelInfoPanel();

private:
	Ui_StudioModelModelInfoPanel _ui;
	StudioModelAsset* const _asset;
};
}
