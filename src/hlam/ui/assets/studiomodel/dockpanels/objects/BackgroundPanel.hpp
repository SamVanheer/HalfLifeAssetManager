#pragma once

#include <QWidget>

#include "ui_BackgroundPanel.h"

namespace studiomodel
{
class StudioModelAsset;

class BackgroundPanel final : public QWidget
{
public:
	BackgroundPanel(StudioModelAsset* asset);
	~BackgroundPanel() = default;

private slots:
	void OnShowBackgroundChanged();

	void OnTextureChanged();
	void OnBrowseTexture();

private:
	Ui_BackgroundPanel _ui;
	StudioModelAsset* const _asset;
};
}
