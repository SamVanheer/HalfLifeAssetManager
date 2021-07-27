#pragma once

#include <QWidget>

#include "ui_StudioModelGroundPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelGroundPanel final : public QWidget
{
public:
	StudioModelGroundPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelGroundPanel() = default;

private slots:
	void OnShowGroundChanged();
	void OnMirrorOnGroundChanged();

	void OnEnableGroundTextureTilingChanged();
	void OnGroundTextureSizeChanged();

	void OnTextureChanged();
	void OnBrowseTexture();

private:
	Ui_StudioModelGroundPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
