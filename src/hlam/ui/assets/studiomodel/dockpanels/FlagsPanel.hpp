#pragma once

#include "ui_FlagsPanel.h"

#include "ui/DockableWidget.hpp"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelData;

class FlagsPanel final : public DockableWidget
{
public:
	FlagsPanel(StudioModelAsset* asset);
	~FlagsPanel() = default;

private:
	void SetFlags(int flags);

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnFlagChanged(int state);

private:
	Ui_FlagsPanel _ui;
	StudioModelAsset* const _asset;
	StudioModelData* _previousModelData{};
};
}
