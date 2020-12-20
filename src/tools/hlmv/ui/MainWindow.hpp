#pragma once

#include <memory>
#include <utility>

#include <QMainWindow>
#include <QPointer>
#include <QString>
#include <QTabWidget>
#include <QUndoGroup>

#include "ui_MainWindow.h"

namespace ui
{
class EditorContext;
class FullscreenWidget;

namespace assets
{
class Asset;
}

namespace settings
{
class GameConfiguration;
class GameEnvironment;
}

class MainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(EditorContext* editorContext);
	~MainWindow();

	bool TryLoadAsset(const QString& fileName);

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	void UpdateTitle(const QString& fileName, bool hasUnsavedChanges);

	void SetupFileSystem(std::pair<settings::GameEnvironment*, settings::GameConfiguration*> activeConfiguration);

private slots:
	void OnOpenLoadAssetDialog();

	void OnAssetCleanChanged(bool clean);

	void OnAssetTabChanged(int index);

	void OnAssetTabCloseRequested(int index);

	void OnAssetFileNameChanged(const QString& fileName);

	void OnSaveAsset();

	void OnSaveAssetAs();

	void OnRecentFilesChanged();

	void OnOpenRecentFile();

	void OnExit();

	void OnGoFullscreen();

	void OnOpenOptionsDialog();

	void OnShowAbout();

	void OnActiveConfigurationChanged(std::pair<settings::GameEnvironment*, settings::GameConfiguration*> current,
		std::pair<settings::GameEnvironment*, settings::GameConfiguration*> previous);

	void OnGameConfigurationDirectoryChanged();

private:
	Ui_MainWindow _ui;

	EditorContext* const _editorContext;

	QUndoGroup* const _undoGroup = new QUndoGroup(this);

	QTabWidget* _assetTabs;

	QPointer<assets::Asset> _currentAsset;

	std::unique_ptr<FullscreenWidget> _fullscreenWidget;
};
}
