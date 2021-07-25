#pragma once

#include <QWidget>

#include "ui_StudioModelFlagsPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelFlagsPanel final : public QWidget
{
public:
	StudioModelFlagsPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelFlagsPanel() = default;

private:
	void SetFlags(int flags);

private slots:
	void InitializeUI();

	void OnModelChanged(const ModelChangeEvent& event);

	void OnFlagChanged(int state);

private:
	Ui_StudioModelFlagsPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
