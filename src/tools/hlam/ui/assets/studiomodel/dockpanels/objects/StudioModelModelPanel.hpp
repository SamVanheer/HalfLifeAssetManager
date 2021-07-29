#pragma once

#include <QBoxLayout>
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

public slots:
	void OnLayoutDirectionChanged(QBoxLayout::Direction direction);

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
