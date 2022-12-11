#pragma once

#include <QWidget>

#include "ui_FlagsPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class FlagsPanel final : public QWidget
{
public:
	FlagsPanel(StudioModelAsset* asset);
	~FlagsPanel() = default;

private:
	void SetFlags(int flags);

private slots:
	void InitializeUI();

	void OnModelChanged(const ModelChangeEvent& event);

	void OnFlagChanged(int state);

private:
	Ui_FlagsPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
