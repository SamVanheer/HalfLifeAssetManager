#pragma once

#include <QBoxLayout>
#include <QString>
#include <QWidget>

#include "ui_ScenePanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;

class ScenePanel final : public DockableWidget
{
	Q_OBJECT

public:
	ScenePanel(StudioModelAsset* asset);
	~ScenePanel() = default;

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

private:
	void InitializeUI();

	void AddObject(QWidget* widget, const QString& label);

signals:
	void LayoutDirectionChanged(QBoxLayout::Direction direction);

private:
	Ui_ScenePanel _ui;
	StudioModelAsset* const _asset;
};
}
}
