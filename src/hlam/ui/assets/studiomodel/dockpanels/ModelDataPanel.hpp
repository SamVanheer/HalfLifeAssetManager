#pragma once

#include "ui_ModelDataPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;
class StudioModelData;

class ModelDataPanel final : public DockableWidget
{
public:
	ModelDataPanel(StudioModelAsset* asset);
	~ModelDataPanel();

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnEyePositionChanged();
	void OnBBoxChanged();
	void OnCBoxChanged();

private:
	Ui_ModelDataPanel _ui;
	StudioModelAsset* const _asset;
	StudioModelData* _previousModelData{};

	bool _changingDataProperties{false};
};
}
}
