#pragma once

#include <QString>
#include <QWidget>

#include "ui_LightingPanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomodel
{
class StudioModelAssetProvider;

class LightingPanel final : public DockableWidget
{
public:
	explicit LightingPanel(StudioModelAssetProvider* provider);
	~LightingPanel();

private:
	void AddLight(const QString& name, QWidget* settingsPanel);

private:
	Ui_LightingPanel _ui;
	StudioModelAssetProvider* const _provider;
};
}
