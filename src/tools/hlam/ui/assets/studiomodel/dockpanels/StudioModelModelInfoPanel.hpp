#pragma once

#include <QWidget>

#include "ui_StudioModelModelInfoPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;

class StudioModelModelInfoPanel final : public QWidget
{
public:
	StudioModelModelInfoPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelModelInfoPanel();

private:
	void InitializeUI();

private slots:
	void OnLoadSnapshot(StateSnapshot* snapshot);

private:
	Ui_StudioModelModelInfoPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
