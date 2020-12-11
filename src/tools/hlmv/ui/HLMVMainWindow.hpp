#pragma once

#include <memory>

#include <QMainWindow>
#include <QString>
#include <QTabWidget>

#include "ui_HLMVMainWindow.h"

namespace ui
{
class EditorContext;
class FullscreenWidget;

class HLMVMainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	HLMVMainWindow(EditorContext* editorContext);
	~HLMVMainWindow();

	void TryLoadAsset(const QString& fileName);

private slots:
	void OnOpenLoadAssetDialog();

	void OnGoFullscreen();

	void OnOpenOptionsDialog();

	void OnShowAbout();

	void OnAssetTabCloseRequested(int index);

private:
	Ui_HLMVMainWindow _ui;

	EditorContext* const _editorContext;

	QTabWidget* _assetTabs;

	std::unique_ptr<FullscreenWidget> _fullscreenWidget;
};
}
