#pragma once

#include <QBoxLayout>
#include <QString>
#include <QWidget>

#include "ui_ScenePanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class ScenePanel final : public QWidget
{
	Q_OBJECT

public:
	ScenePanel(StudioModelAsset* asset);
	~ScenePanel() = default;

private:
	void InitializeUI();

	void AddObject(QWidget* widget, const QString& label);

signals:
	void LayoutDirectionChanged(QBoxLayout::Direction direction);

public slots:
	void OnLayoutDirectionChanged();

private:
	Ui_ScenePanel _ui;
	StudioModelAsset* const _asset;
};
}
}
