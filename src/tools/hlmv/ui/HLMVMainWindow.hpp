#pragma once

#include <memory>

#include <QMainWindow>
#include <QString>
#include <QTabWidget>

#include "ui_HLMVMainWindow.h"

#include "ui/assets/Assets.hpp"

namespace ui
{
class EditorUIContext;

namespace assets
{
class IAsset;
class IAssetProviderRegistry;
}

class HLMVMainWindow final : public QMainWindow
{
	Q_OBJECT

public:
	HLMVMainWindow(std::unique_ptr<assets::IAssetProviderRegistry>&& assetProviderRegistry);
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

	std::unique_ptr<assets::IAssetProviderRegistry> _assetProviderRegistry;
};
}
