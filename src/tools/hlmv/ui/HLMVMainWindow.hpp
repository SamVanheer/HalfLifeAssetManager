#pragma once

#include <memory>

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

class HLMVMainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	HLMVMainWindow(EditorContext* editorContext);
	~HLMVMainWindow();

	void TryLoadAsset(const QString& fileName);

private:
	void UpdateTitle(const QString& fileName, bool hasUnsavedChanges);

private slots:
	void OnAssetCleanChanged(bool clean);

	void OnAssetTabChanged(int index);

	void OnAssetTabCloseRequested(int index);

	void OnAssetFileNameChanged(const QString& fileName);

	void OnOpenLoadAssetDialog();

	void OnGoFullscreen();

	void OnOpenOptionsDialog();

	void OnShowAbout();

private:
	Ui_HLMVMainWindow _ui;

	EditorContext* const _editorContext;

	QUndoGroup* const _undoGroup = new QUndoGroup(this);

	QTabWidget* _assetTabs;

	std::unique_ptr<FullscreenWidget> _fullscreenWidget;
};
}
