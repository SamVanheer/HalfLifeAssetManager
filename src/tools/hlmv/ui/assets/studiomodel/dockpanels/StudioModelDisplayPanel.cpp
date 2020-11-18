#include "ui/assets/studiomodel/dockpanels/StudioModelDisplayPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelDisplayPanel::StudioModelDisplayPanel(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);
}

StudioModelDisplayPanel::~StudioModelDisplayPanel() = default;
}
