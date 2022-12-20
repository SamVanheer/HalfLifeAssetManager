#pragma once

#include <QWidget>

#include "ui_SkyLightPanel.h"

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;

class SkyLightPanel final : public QWidget
{
public:
	explicit SkyLightPanel(StudioModelAssetProvider* provider);
	~SkyLightPanel();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnAnglesChanged();

private:
	Ui_SkyLightPanel _ui;
	StudioModelAssetProvider* const _provider;
};
}
