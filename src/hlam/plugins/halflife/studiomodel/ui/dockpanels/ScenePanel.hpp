#pragma once

#include <QBoxLayout>
#include <QString>
#include <QWidget>

#include "ui_ScenePanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;

class ScenePanel final : public DockableWidget
{
public:
	explicit ScenePanel(StudioModelAssetProvider* provider);
	~ScenePanel() = default;

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

private:
	void InitializeUI();

	void AddObject(QWidget* widget, const QString& label);

private:
	Ui_ScenePanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
};
}
