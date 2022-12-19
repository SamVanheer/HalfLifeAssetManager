#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "ui_GroundPanel.h"

namespace studiomodel
{
class StudioModelAssetProvider;

class GroundPanel final : public QWidget
{
public:
	explicit GroundPanel(StudioModelAssetProvider* provider);
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
	StudioModelAssetProvider* const _provider;
};
}
