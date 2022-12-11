#pragma once

#include "ui_ModelInfoPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;

class ModelInfoPanel final : public DockableWidget
{
public:
	ModelInfoPanel(StudioModelAsset* asset);
	~ModelInfoPanel();

private slots:
	void InitializeUI();

private:
	Ui_ModelInfoPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
