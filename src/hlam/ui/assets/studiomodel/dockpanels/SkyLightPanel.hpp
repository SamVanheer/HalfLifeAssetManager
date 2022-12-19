#pragma once

#include <QWidget>

#include "ui_SkyLightPanel.h"

namespace studiomodel
{
class StudioModelAssetProvider;

class SkyLightPanel final : public QWidget
{
public:
	explicit SkyLightPanel(StudioModelAssetProvider* provider);
	~SkyLightPanel();

private slots:
	void OnAnglesChanged();

private:
	Ui_SkyLightPanel _ui;
	StudioModelAssetProvider* const _provider;
};
}
