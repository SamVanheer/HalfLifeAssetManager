#pragma once

#include "ui_ModelDataPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class ModelDataPanel final : public DockableWidget
{
public:
	ModelDataPanel(StudioModelAsset* asset);
	~ModelDataPanel();

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnAssetChanged(StudioModelAsset* asset);

	void OnEyePositionChanged(const glm::vec3& value);

	void OnBBoxMinChanged(const glm::vec3& value);
	void OnBBoxMaxChanged(const glm::vec3& value);

	void OnCBoxMinChanged(const glm::vec3& value);
	void OnCBoxMaxChanged(const glm::vec3& value);

private:
	Ui_ModelDataPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
