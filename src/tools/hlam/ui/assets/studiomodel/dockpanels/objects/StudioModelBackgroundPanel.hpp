#pragma once

#include <QWidget>

#include "ui_StudioModelBackgroundPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelBackgroundPanel final : public QWidget
{
public:
	StudioModelBackgroundPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelBackgroundPanel() = default;

private slots:
	void OnShowBackgroundChanged();

	void OnTextureChanged();
	void OnBrowseTexture();

private:
	Ui_StudioModelBackgroundPanel _ui;
	StudioModelAsset* const _asset;
};
}
