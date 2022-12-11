#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "ui_ModelPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;

class ModelPanel final : public QWidget
{
public:
	ModelPanel(StudioModelAsset* asset);
	~ModelPanel() = default;

public slots:
	void OnLayoutDirectionChanged(QBoxLayout::Direction direction);

private slots:
	void OnOriginChanged();

	void OnCenterOnWorldOrigin();
	void OnAlignOnGround();

private:
	Ui_ModelPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
