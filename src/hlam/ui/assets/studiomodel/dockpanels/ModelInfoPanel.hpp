#pragma once

#include "ui_ModelInfoPanel.h"

#include "ui/DockableWidget.hpp"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelData;

class ModelInfoPanel final : public DockableWidget
{
public:
	ModelInfoPanel(StudioModelAsset* asset);
	~ModelInfoPanel();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

private:
	Ui_ModelInfoPanel _ui;
	StudioModelAsset* const _asset;
	StudioModelData* _previousModelData{};
};
}
