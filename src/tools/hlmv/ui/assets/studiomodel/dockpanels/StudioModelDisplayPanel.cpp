#include "ui/assets/studiomodel/dockpanels/StudioModelDisplayPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelDisplayPanel::StudioModelDisplayPanel(StudioModelContext* context, QWidget* parent)
	: QWidget(parent)
	, _context(context)
{
	_ui.setupUi(this);
}

StudioModelDisplayPanel::~StudioModelDisplayPanel() = default;
}
