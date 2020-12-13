#pragma once

#include <QWidget>

#include "ui_StudioModelGlobalFlagsPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelGlobalFlagsPanel final : public QWidget
{
public:
	StudioModelGlobalFlagsPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelGlobalFlagsPanel();

private slots:
	void OnFlagChanged(int state);

private:
	Ui_StudioModelGlobalFlagsPanel _ui;
	StudioModelAsset* const _asset;
};
}
