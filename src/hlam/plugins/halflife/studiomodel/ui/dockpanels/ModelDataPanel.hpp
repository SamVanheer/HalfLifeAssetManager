#pragma once

#include "ui_ModelDataPanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

class ModelDataPanel final : public DockableWidget
{
public:
	explicit ModelDataPanel(StudioModelAssetProvider* provider);
	~ModelDataPanel();

private:
	void SetFlags(int flags);

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnEyePositionChanged();
	void OnBBoxChanged();
	void OnCBoxChanged();

	void OnFlagChanged(int state);

private:
	Ui_ModelDataPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};

	bool _changingDataProperties{false};
};
}
