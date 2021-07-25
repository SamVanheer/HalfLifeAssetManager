#pragma once

#include <QWidget>

#include <glm/vec3.hpp>

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

private:
	void InitializeUI();

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnOriginChanged();

	void OnScaleMesh();

	void OnScaleBones();

	void OnEyePositionChanged(const glm::vec3& value);

	void OnBBoxMinChanged(const glm::vec3& value);
	void OnBBoxMaxChanged(const glm::vec3& value);

	void OnCBoxMinChanged(const glm::vec3& value);
	void OnCBoxMaxChanged(const glm::vec3& value);

private:
	Ui_StudioModelModelDataPanel _ui;
	StudioModelAsset* const _asset;

	glm::vec3 _oldOffset{0};
};
}
}
