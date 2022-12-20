#pragma once

#include <memory>
#include <utility>

#include <QMainWindow>
#include <QPointer>
#include <QString>
#include <QTabWidget>
#include <QUndoGroup>

#include "ui_MainWindow.h"

class Asset;
class EditorContext;
class FileListPanel;
class FullscreenWidget;
class GameConfiguration;
class GameEnvironment;
class QActionGroup;

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

	bool VerifyNoUnsavedChanges(Asset* asset);

	bool TryCloseAsset(int index, bool verifyUnsavedChanges);

	void UpdateTitle(const QString& fileName, bool hasUnsavedChanges);

	void SetupFileSystem(std::pair<GameEnvironment*, GameConfiguration*> activeConfiguration);

private slots:
	bool TryLoadAsset(QString fileName);

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

	void OnGoFullscreen();

	void OnFileSelected(const QString& fileName);

	void OnTextureFiltersChanged();

	void OnRefreshAsset();

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

	QActionGroup* _msaaActionGroup;

	QUndoGroup* const _undoGroup = new QUndoGroup(this);

	QPointer<QTabWidget> _assetTabs;

	QString _loadFileFilter;
	QString _saveFileFilter;

	QPointer<Asset> _currentAsset;

	std::unique_ptr<FullscreenWidget> _fullscreenWidget;

	QPointer<QDockWidget> _fileListDock;
};
