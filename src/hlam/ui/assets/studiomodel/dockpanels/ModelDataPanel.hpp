#pragma once

#include <QWidget>

#include "ui_ModelDataPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class ModelDataPanel final : public QWidget
{
public:
	ModelDataPanel(StudioModelAsset* asset);
	~ModelDataPanel();

public slots:
	void OnLayoutDirectionChanged();

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void InitializeUI();

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
