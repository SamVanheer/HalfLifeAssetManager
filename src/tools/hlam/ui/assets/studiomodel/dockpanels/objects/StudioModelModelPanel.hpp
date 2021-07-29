#pragma once

#include <QWidget>

#include "ui_StudioModelModelPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;

class StudioModelModelPanel final : public QWidget
{
public:
	StudioModelModelPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelModelPanel() = default;

private slots:
	void OnOriginChanged();

	void OnCenterOnWorldOrigin();
	void OnAlignOnGround();

private:
	Ui_StudioModelModelPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
