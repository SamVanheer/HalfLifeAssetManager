#pragma once

#include "ui_ModelInfoPanel.h"

#include "ui/DockableWidget.hpp"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

class ModelInfoPanel final : public DockableWidget
{
public:
	explicit ModelInfoPanel(StudioModelAssetProvider* provider);
	~ModelInfoPanel();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

private:
	Ui_ModelInfoPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};
};
}
