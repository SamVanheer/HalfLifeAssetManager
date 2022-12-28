#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QTabWidget>
#include <QUndoGroup>

#include "ui_MainWindow.h"

class Asset;
class AssetProvider;
class EditorContext;
class FileListPanel;
class FullscreenWidget;
class GameConfiguration;
class GameEnvironment;
class QActionGroup;

enum class LoadResult
{
	Success = 0,
	Failed,
	Cancelled
};

class MainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(EditorContext* editorContext);
	~MainWindow();

	void LoadSettings();

protected:
	void closeEvent(QCloseEvent* event) override;

	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Asset* GetAsset(int index) const;
	Asset* GetCurrentAsset() const;

	bool SaveAsset(Asset* asset);

	bool VerifyNoUnsavedChanges(Asset* asset, bool allowCancel);

	bool TryCloseAsset(int index, bool verifyUnsavedChanges, bool allowCancel = true);

	void UpdateTitle(const QString& fileName, bool hasUnsavedChanges);

	void SetupFileSystem(std::pair<GameEnvironment*, GameConfiguration*> activeConfiguration);

private slots:
	LoadResult TryLoadAsset(QString fileName);

	void SyncSettings();

	void OnOpenLoadAssetDialog();

	void OnAssetCleanChanged(bool clean);

	void OnAssetTabChanged(int index);

	void OnAssetTabCloseRequested(int index);

	void OnAssetFileNameChanged(const QString& fileName);

	void OnSaveAsset();

	void OnSaveAssetAs();

	void OnCloseAsset();

	void OnRecentFilesChanged();

	void OnOpenRecentFile();

	void OnExit();

	void OnEnterFullscreen();

	void OnExitFullscreen();

	void OnFileSelected(const QString& fileName);

	void OnTextureFiltersChanged();

	bool OnRefreshAsset();

	void OnPlaySoundsChanged();

	void OnFramerateAffectsPitchChanged();

	void OnOpenOptionsDialog();

	void OnShowAbout();

	void OnActiveConfigurationChanged(std::pair<GameEnvironment*, GameConfiguration*> current,
		std::pair<GameEnvironment*, GameConfiguration*> previous);

	void OnGameConfigurationDirectoryChanged();

private:
	Ui_MainWindow _ui;

	EditorContext* const _editorContext;

	QMap<AssetProvider*, QMenu*> _assetMenus;

	QPointer<QMenu> _assetMenu;

	QActionGroup* _msaaActionGroup;

	QUndoGroup* const _undoGroup = new QUndoGroup(this);

	QPointer<QTabWidget> _assetTabs;

	QString _loadFileFilter;
	QString _saveFileFilter;

	std::vector<std::unique_ptr<Asset>> _assets;
	QPointer<Asset> _currentAsset;

	std::unique_ptr<FullscreenWidget> _fullscreenWidget;

	QPointer<QDockWidget> _fileListDock;
};
