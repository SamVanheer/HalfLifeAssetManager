#pragma once

#include <QWidget>

#include "ui_BackgroundPanel.h"

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;

class BackgroundPanel final : public QWidget
{
public:
	explicit BackgroundPanel(StudioModelAssetProvider* provider);
	~BackgroundPanel() = default;

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnShowBackgroundChanged();

	void OnTextureChanged();
	void OnBrowseTexture();

private:
	Ui_BackgroundPanel _ui;
	StudioModelAssetProvider* const _provider;
};
}
