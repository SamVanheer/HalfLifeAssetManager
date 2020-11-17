#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

namespace ui::assets::studiomodel
{
StudioModelEditWidget::StudioModelEditWidget(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
}

StudioModelEditWidget::~StudioModelEditWidget() = default;
}
