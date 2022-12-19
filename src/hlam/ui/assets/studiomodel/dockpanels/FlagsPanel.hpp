#pragma once

#include "ui_FlagsPanel.h"

#include "ui/DockableWidget.hpp"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

class FlagsPanel final : public DockableWidget
{
public:
	explicit FlagsPanel(StudioModelAssetProvider* provider);
	~FlagsPanel() = default;

private:
	void SetFlags(int flags);

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnFlagChanged(int state);

private:
	Ui_FlagsPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};
};
}
