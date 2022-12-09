#pragma once

#include <QWidget>

#include "ui_StudioModelTransformPanel.h"

class QAction;

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelTransformPanel : public QWidget
{
public:
	StudioModelTransformPanel(StudioModelAsset* asset);
	~StudioModelTransformPanel() = default;

public slots:
	void ResetValues();

private slots:
	void OnApply();

private:
	StudioModelAsset* const _asset;

	Ui_StudioModelTransformPanel _ui;
};
}
