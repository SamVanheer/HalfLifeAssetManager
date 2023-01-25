#pragma once

#include <memory>

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QString>

#include "ui_MainWindow.h"

class Asset;
class AssetList;
class AssetProvider;
class AssetManager;
class FullscreenWidget;
class QActionGroup;
class QGridLayout;
class QMenu;
class QStringList;
class QTabBar;
class QToolButton;
class QUndoGroup;
class QWidget;

class MainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(AssetManager* application);
	~MainWindow();

	void LoadSettings();

	void LoadFile(const QString& fileName);

protected:
	void closeEvent(QCloseEvent* event) override;

	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void UpdateTitle(const QString& fileName, bool hasUnsavedChanges);

	void MaybeOpenAll(const QStringList& fileNames);

	void TryLoadInExternalProgram(const QString& fileName, const QString& externalProgramKey);

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

	QUndoGroup* const _undoGroup;

	QWidget* _assetsWidget;
	QGridLayout* _assetsLayout;
	QPointer<QTabBar> _assetTabs;
	QToolButton* _assetListButton;
	QMenu* _assetListMenu;
	QPointer<QWidget> _currentEditWidget;

	bool _activateNewTabs = true;
	bool _modifyingTabs = false;

	QString _loadFileFilter;
	QString _saveFileFilter;

	std::unique_ptr<FullscreenWidget> _fullscreenWidget;

	QPointer<QDockWidget> _fileBrowserDock;
};
