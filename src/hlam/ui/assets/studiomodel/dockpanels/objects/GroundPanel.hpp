#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "ui_GroundPanel.h"

namespace studiomodel
{
class StudioModelAsset;

class GroundPanel final : public QWidget
{
public:
	GroundPanel(StudioModelAsset* asset);
	~GroundPanel() = default;

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
	Ui_GroundPanel _ui;
	StudioModelAsset* const _asset;
};
}
