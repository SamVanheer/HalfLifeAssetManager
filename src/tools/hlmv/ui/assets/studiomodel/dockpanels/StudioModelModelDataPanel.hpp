#pragma once

#include <QWidget>

#include "ui_StudioModelModelDataPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelModelDataPanel final : public QWidget
{
public:
	StudioModelModelDataPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelModelDataPanel();

private:
	void UpdateOrigin();

private slots:
	void OnOriginChanged();

	void OnSetOrigin();

	void OnScaleMesh();

	void OnScaleBones();

private:
	Ui_StudioModelModelDataPanel _ui;
	StudioModelAsset* const _asset;
};
}
