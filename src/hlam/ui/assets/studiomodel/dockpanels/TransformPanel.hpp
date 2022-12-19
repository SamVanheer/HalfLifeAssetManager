#pragma once

#include "ui_TransformPanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;

class TransformPanel : public DockableWidget
{
public:
	explicit TransformPanel(StudioModelAssetProvider* provider);
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
	Ui_TransformPanel _ui;
	StudioModelAssetProvider* const _provider;
};
}
