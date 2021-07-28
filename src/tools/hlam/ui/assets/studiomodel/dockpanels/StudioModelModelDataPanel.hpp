#pragma once

#include <QWidget>

#include "ui_StudioModelModelDataPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelModelDataPanel final : public QWidget
{
public:
	StudioModelModelDataPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelModelDataPanel();

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
	Ui_StudioModelModelDataPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
