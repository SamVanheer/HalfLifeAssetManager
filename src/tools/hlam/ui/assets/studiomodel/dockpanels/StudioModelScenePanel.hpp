#pragma once

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
public:
	StudioModelScenePanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelScenePanel() = default;

private:
	void InitializeUI();

	void AddObject(QWidget* widget, const QString& label);

private:
	Ui_StudioModelScenePanel _ui;
	StudioModelAsset* const _asset;
};
}
}
