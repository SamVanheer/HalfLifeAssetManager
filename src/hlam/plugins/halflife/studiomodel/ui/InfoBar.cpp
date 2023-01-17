#include <chrono>

#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/Scene.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/InfoBar.hpp"

using namespace std::chrono;

namespace studiomodel
{
InfoBar::InfoBar(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);
}

InfoBar::~InfoBar() = default;

void InfoBar::Tick()
{
	const long long currentTick = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

	if (_lastFPSUpdate == 0 || ((currentTick - _lastFPSUpdate) >= 1000))
	{
		_lastFPSUpdate = currentTick;

		_ui.FPSLabel->setText(QString::number(_currentFPS));

		_currentFPS = 0;
	}
}

void InfoBar::SetAsset(StudioModelAsset* asset)
{
	if (_asset == asset)
	{
		return;
	}

	_asset = asset;
	_oldDrawnPolygonsCount = 0;
}

void InfoBar::OnDraw()
{
	++_currentFPS;

	const unsigned int drawnPolygonsCount = _asset ? _asset->GetCurrentScene()->GetDrawnPolygonsCount() : 0;

	//Don't update if it's identical. Prevents flickering
	if (_oldDrawnPolygonsCount != drawnPolygonsCount)
	{
		_oldDrawnPolygonsCount = drawnPolygonsCount;
		_ui.DrawnPolygonsCountLabel->setText(QString::number(drawnPolygonsCount));
	}
}
}
