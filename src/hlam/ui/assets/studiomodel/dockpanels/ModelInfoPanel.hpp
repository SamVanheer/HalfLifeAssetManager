#pragma once

#include <QWidget>

#include "ui_ModelInfoPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;

class ModelInfoPanel final : public QWidget
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
