#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "ui_StudioModelGroundPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelGroundPanel final : public QWidget
{
public:
	StudioModelGroundPanel(StudioModelAsset* asset);
	~StudioModelGroundPanel() = default;

public slots:
	void OnLayoutDirectionChanged(QBoxLayout::Direction direction);

private slots:
	void OnShowGroundChanged();
	void OnMirrorOnGroundChanged();

	void OnEnableGroundTextureTilingChanged();
	void OnGroundTextureSizeChanged();

	void OnTextureChanged();
	void OnBrowseTexture();

	void OnOriginChanged();

private:
	Ui_StudioModelGroundPanel _ui;
	StudioModelAsset* const _asset;
};
}
