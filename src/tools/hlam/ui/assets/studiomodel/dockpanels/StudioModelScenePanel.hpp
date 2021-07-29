#pragma once

#include <QBoxLayout>
#include <QString>
#include <QWidget>

#include "ui_StudioModelScenePanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelScenePanel final : public QWidget
{
	Q_OBJECT

public:
	StudioModelScenePanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelScenePanel() = default;

private:
	void InitializeUI();

	void AddObject(QWidget* widget, const QString& label);

signals:
	void LayoutDirectionChanged(QBoxLayout::Direction direction);

public slots:
	void OnLayoutDirectionChanged();

private:
	Ui_StudioModelScenePanel _ui;
	StudioModelAsset* const _asset;
};
}
}
