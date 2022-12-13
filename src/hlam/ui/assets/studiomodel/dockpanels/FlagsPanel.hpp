#pragma once

#include "ui_FlagsPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class FlagsPanel final : public DockableWidget
{
public:
	FlagsPanel(StudioModelAsset* asset);
	~FlagsPanel() = default;

private:
	void SetFlags(int flags);

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnAssetChanged(StudioModelAsset* asset);

	void OnFlagChanged(int state);

private:
	Ui_FlagsPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
