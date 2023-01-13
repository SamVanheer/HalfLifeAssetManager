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
class AssetList;
class AssetProvider;
class AssetManager;
class FullscreenWidget;
class QActionGroup;
class QMenu;
class QToolButton;

class MainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(AssetManager* application);
	~MainWindow();

	void LoadSettings();

protected:
	void closeEvent(QCloseEvent* event) override;

	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void UpdateTitle(const QString& fileName, bool hasUnsavedChanges);

	void CloseAllButCount(int leaveOpenCount, bool verifyUnsavedChanges);

	QString GetCanonicalFileName(Asset* asset) const;

	bool ShouldContinueWithSave(Asset* asset);

private slots:
	void SyncSettings();

	void OnOpenLoadAssetDialog();

	void OnAssetTabChanged(int index);

	void OnAssetAdded(int index);

	void OnAssetActivated();

	void OnAboutToCloseAsset(int index);

	void OnAboutToRemoveAsset(int index);

	void OnAssetRemoved(int index);

	void OnAssetFileNameChanged(Asset* asset);

	void OnSaveAsset();

	void OnSaveAssetAs();

	void OnRecentFilesChanged();

	void OnOpenRecentFile();

	void OnTextureFiltersChanged();

	void OnOpenOptionsDialog();

private:
	Ui_MainWindow _ui;

	AssetManager* const _application;
	AssetList* const _assets;

	QMap<AssetProvider*, QMenu*> _assetMenus;

	QPointer<QMenu> _assetMenu;

	QActionGroup* _msaaActionGroup;

	QUndoGroup* const _undoGroup = new QUndoGroup(this);

	QPointer<QTabWidget> _assetTabs;
	QToolButton* _assetListButton;
	QMenu* _assetListMenu;

	bool _activateNewTabs = true;
	bool _modifyingTabs = false;

	QString _loadFileFilter;
	QString _saveFileFilter;

	std::unique_ptr<FullscreenWidget> _fullscreenWidget;

	QPointer<QDockWidget> _fileBrowserDock;
};
