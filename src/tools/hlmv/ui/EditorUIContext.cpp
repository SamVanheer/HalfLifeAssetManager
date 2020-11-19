#include "ui/EditorUIContext.hpp"

#include "ui/assets/Assets.hpp"

namespace ui
{
LoadedAsset::LoadedAsset(std::unique_ptr<assets::IAsset>&& asset, QWidget* editWidget)
	: Asset(std::move(asset))
	, EditWidget(editWidget)
{
}

LoadedAsset::~LoadedAsset() = default;

EditorUIContext::EditorUIContext(QObject* parent)
	: QObject(parent)
	, _timer(new QTimer(this))
{
	connect(_timer, &QTimer::timeout, this, &EditorUIContext::OnTimerTick);
}

EditorUIContext::~EditorUIContext() = default;

void EditorUIContext::OnTimerTick()
{
	//TODO: update frequency should be controllable
	emit Tick();
}
}
