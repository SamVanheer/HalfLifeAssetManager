#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "ui_GroundPanel.h"

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;

class GroundPanel final : public QWidget
{
public:
	explicit GroundPanel(StudioModelAssetProvider* provider);
	~GroundPanel() = default;

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnShowGroundChanged();
	void OnMirrorOnGroundChanged();

	void OnEnableTextureChanged();

	void OnEnableGroundTextureTilingChanged();
	void OnGroundTextureSizeChanged();

	void OnTextureChanged();
	void OnBrowseTexture();

	void OnOriginChanged();

private:
	Ui_GroundPanel _ui;
	StudioModelAssetProvider* const _provider;
};
}
