#pragma once

#include <QWidget>

#include "ui_SkyLightPanel.h"

class QColor;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;

class SkyLightPanel final : public QWidget
{
public:
	explicit SkyLightPanel(StudioModelAssetProvider* provider);
	~SkyLightPanel();

private:
	void SetButtonColor(const QColor& color);

public slots:
	void OnLayoutDirectionChanged(QBoxLayout::Direction direction);

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnAnglesChanged();

	void OnSelectColor();

	void OnAmbientChanged(int value);

	void OnShadeChanged(int value);

private:
	Ui_SkyLightPanel _ui;
	StudioModelAssetProvider* const _provider;
};
}
