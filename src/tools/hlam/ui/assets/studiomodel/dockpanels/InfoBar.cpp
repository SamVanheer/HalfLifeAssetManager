#include <chrono>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/InfoBar.hpp"

namespace ui::assets::studiomodel
{
InfoBar::InfoBar(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);
}

InfoBar::~InfoBar() = default;

void InfoBar::OnDraw()
{
	++_currentFPS;

	const long long currentTick = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	if (_lastFPSUpdate == 0 || ((currentTick - _lastFPSUpdate) >= 1000))
	{
		_lastFPSUpdate = currentTick;

		_ui.FPSLabel->setText(QString::number(_currentFPS));

		_currentFPS = 0;
	}

	const unsigned int drawnPolygonsCount = _asset->GetScene()->GetDrawnPolygonsCount();

	//Don't update if it's identical. Prevents flickering
	if (_oldDrawnPolygonsCount != drawnPolygonsCount)
	{
		_oldDrawnPolygonsCount = drawnPolygonsCount;
		_ui.DrawnPolygonsCountLabel->setText(QString::number(drawnPolygonsCount));
	}
}
}
