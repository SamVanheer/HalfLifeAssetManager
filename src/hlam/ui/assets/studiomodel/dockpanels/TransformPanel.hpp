#pragma once

#include "ui_TransformPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class TransformPanel : public DockableWidget
{
public:
	TransformPanel(StudioModelAsset* asset);
	~TransformPanel() = default;

	void OnVisibilityChanged(bool visible) override
	{
		ResetValues();
	}

public slots:
	void ResetValues();

private slots:
	void OnApply();

private:
	StudioModelAsset* const _asset;

	Ui_TransformPanel _ui;
};
}
