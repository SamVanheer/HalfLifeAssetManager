#pragma once

#include <QWidget>

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelEditWidget final : public QWidget
{
public:
	StudioModelEditWidget(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelEditWidget();

private:
	StudioModelAsset* _asset;
};
}
