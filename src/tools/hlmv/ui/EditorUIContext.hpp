#pragma once

#include <memory>
#include <vector>

#include <QObject>
#include <QTimer>

namespace filesystem
{
class IFileSystem;
}

namespace soundsystem
{
class ISoundSystem;
}

namespace ui
{
namespace assets
{
class IAsset;
}

class LoadedAsset
{
public:

	LoadedAsset(std::unique_ptr<assets::IAsset>&& asset, QWidget* editWidget);
	~LoadedAsset();

	LoadedAsset(const LoadedAsset&) = delete;
	LoadedAsset& operator=(const LoadedAsset&) = delete;
	LoadedAsset(LoadedAsset&&) = default;
	LoadedAsset& operator=(LoadedAsset&&) = default;

	assets::IAsset* GetAsset() const { return Asset.get(); }

	QWidget* GetEditWidget() const { return EditWidget; }

private:
	std::unique_ptr<assets::IAsset> Asset;
	QWidget* EditWidget;
};

/**
*	@brief Used to communicate between the main window and edit widgets
*/
class EditorUIContext final : public QObject
{
	Q_OBJECT

public:
	EditorUIContext(QObject* parent = nullptr);
	~EditorUIContext();
	EditorUIContext(const EditorUIContext&) = delete;
	EditorUIContext& operator=(const EditorUIContext&) = delete;

	QTimer* GetTimer() const { return _timer; }

	filesystem::IFileSystem* GetFileSystem() const { return _fileSystem.get(); }

	soundsystem::ISoundSystem* GetSoundSystem() const { return _soundSystem.get(); }

	std::vector<LoadedAsset>& GetLoadedAssets() { return _loadedAssets; }

signals:
	/**
	*	@brief Emitted every time a frame tick occurs
	*/
	void Tick();

private slots:
	void OnTimerTick();

private:
	QTimer* const _timer;

	std::unique_ptr<filesystem::IFileSystem> _fileSystem;
	std::unique_ptr<soundsystem::ISoundSystem> _soundSystem;

	std::vector<LoadedAsset> _loadedAssets;
};
}
