#pragma once

#include <memory>

#include <QMainWindow>
#include <QString>
#include <QTabWidget>

#include "ui_HLMVMainWindow.h"

namespace ui
{
class EditorUIContext;

class HLMVMainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	HLMVMainWindow(EditorUIContext* editorContext);
	~HLMVMainWindow();

private slots:
	void OnOpenLoadAssetDialog();
	void OnOpenOptionsDialog();

	void OnShowAbout();

private:
	void LoadAsset(const QString& fileName);

private:
	Ui_HLMVMainWindow _ui;

	EditorUIContext* const _editorContext;

	QTabWidget* _assetTabs;
};
}
