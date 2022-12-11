#pragma once

#include <QWidget>

#include "ui_TransformPanel.h"

class QAction;

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class TransformPanel : public QWidget
{
public:
	TransformPanel(StudioModelAsset* asset);
	~TransformPanel() = default;

public slots:
	void ResetValues();

private slots:
	void OnApply();

private:
	StudioModelAsset* const _asset;

	Ui_TransformPanel _ui;
};
}
