#pragma once

#include <memory>
#include <utility>

#include <QMainWindow>
#include <QString>
#include <QTabWidget>
#include <QUndoGroup>

#include "ui_HLMVMainWindow.h"

namespace ui
{
class EditorContext;
class FullscreenWidget;

namespace assets
{
class AssetDataChangeEvent;
}

namespace settings
{
class GameConfiguration;
class GameEnvironment;
}

class HLMVMainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	HLMVMainWindow(EditorContext* editorContext);
	~HLMVMainWindow();

	bool TryLoadAsset(const QString& fileName);

private:
	void UpdateTitle(const QString& fileName, bool hasUnsavedChanges);

	void SetupFileSystem(std::pair<settings::GameEnvironment*, settings::GameConfiguration*> activeConfiguration);

private slots:
	void OnRecentFilesChanged();

	void OnOpenRecentFile();

	void OnExit();

	void OnAssetCleanChanged(bool clean);

	void OnAssetTabChanged(int index);

	void OnAssetTabCloseRequested(int index);

	void OnAssetFileNameChanged(const QString& fileName);

	void OnOpenLoadAssetDialog();

	void OnGoFullscreen();

	void OnOpenOptionsDialog();

	void OnShowAbout();

	void OnActiveConfigurationChanged(std::pair<settings::GameEnvironment*, settings::GameConfiguration*> current,
		std::pair<settings::GameEnvironment*, settings::GameConfiguration*> previous);

	void OnGameConfigurationDirectoryChanged();

private:
	Ui_HLMVMainWindow _ui;

	EditorContext* const _editorContext;

	QUndoGroup* const _undoGroup = new QUndoGroup(this);

	QTabWidget* _assetTabs;

	std::unique_ptr<FullscreenWidget> _fullscreenWidget;
};
}
