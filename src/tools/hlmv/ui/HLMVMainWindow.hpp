#pragma once

#include <memory>
#include <vector>

#include <QMainWindow>
#include <QString>
#include <QTabWidget>

#include "ui_HLMVMainWindow.h"

#include "ui/assets/Assets.hpp"

namespace ui
{
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
	struct AssetData
	{
		AssetData(std::unique_ptr<assets::IAsset>&& asset, QWidget* editWidget)
			: Asset(std::move(asset))
			, EditWidget(editWidget)
		{
		}

		std::unique_ptr<assets::IAsset> Asset;
		QWidget* EditWidget;
	};

	Ui_HLMVMainWindow _ui;

	QTabWidget* _assetTabs;

	std::unique_ptr<assets::IAssetProviderRegistry> _assetProviderRegistry;

	std::vector<AssetData> _openAssets;
};
}
