#pragma once

#include <QWidget>

#include <glm/vec3.hpp>

#include "ui_StudioModelModelDataPanel.h"

namespace ui::assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelModelDataPanel final : public QWidget
{
public:
	StudioModelModelDataPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelModelDataPanel();

private:
	void UpdateOrigin();

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnOriginChanged();

	void OnSetOrigin();

	void OnScaleMesh();

	void OnScaleBones();

private:
	Ui_StudioModelModelDataPanel _ui;
	StudioModelAsset* const _asset;

	glm::vec3 _oldOffset{0};
};
}
