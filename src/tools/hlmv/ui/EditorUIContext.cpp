#include <stdexcept>

#include <QMessageBox>

#include "filesystem/CFileSystem.h"
#include "filesystem/IFileSystem.h"

#include "soundsystem/CSoundSystem.h"
#include "soundsystem/ISoundSystem.h"

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
	, _fileSystem(std::make_unique<filesystem::CFileSystem>())
	, _soundSystem(std::make_unique<soundsystem::CSoundSystem>())
{
	//TODO: set up filesystem based on game configuration
	if (!_fileSystem->Initialize())
	{
		QMessageBox::critical(nullptr, "Fatal Error", "Failed to initialize file system");
		//TODO: ensure error handling cleans up properly
		throw std::runtime_error("Failed to initialize file system");
	}

	if (!_soundSystem->Initialize(_fileSystem.get()))
	{
		QMessageBox::critical(nullptr, "Fatal Error", "Failed to initialize sound system");
		throw std::runtime_error("Failed to initialize sound system");
	}

	connect(_timer, &QTimer::timeout, this, &EditorUIContext::OnTimerTick);
}

EditorUIContext::~EditorUIContext()
{
	_soundSystem->Shutdown();
	_fileSystem->Shutdown();
}

void EditorUIContext::OnTimerTick()
{
	//TODO: update frequency should be controllable
	emit Tick();
}
}
