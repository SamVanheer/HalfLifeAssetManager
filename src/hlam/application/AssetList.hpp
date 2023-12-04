#pragma once

#include <memory>
#include <variant>
#include <vector>

#include <QObject>
#include <QPointer>

#include <spdlog/logger.h>

#include "application/Assets.hpp"
#include "qt/ObservableList.hpp"

class AssetManager;

enum class AssetLoadAction
{
	Success = 0,
	Failed,
	Cancelled
};

using AssetLoadResult = std::variant<AssetLoadAction, AssetLoadInExternalProgram>;

/**
*	@brief Manages access to the asset list and handles opening, closing and saving of assets.
*/
class AssetList final : public QObject
{
	Q_OBJECT

public:
	using ObservableAssetList = ObservableList<std::unique_ptr<Asset>>;

	explicit AssetList(AssetManager* application, std::shared_ptr<spdlog::logger> logger);
	~AssetList() override;

	ObservableAssetList* GetObservableList() { return _assetList.get(); }

	std::size_t Count() const;

	int IndexOf(const Asset* asset) const;

	Asset* Get(std::size_t index) const;

	Asset* GetCurrent() const { return _currentAsset; }

	void SetCurrent(Asset* asset);

	AssetLoadResult TryLoad(const QString& fileName);

	bool TryClose(int index, bool verifyUnsavedChanges, bool allowCancel = true);

	bool Save(Asset* asset);

	bool VerifyNoUnsavedChanges(Asset* asset, bool allowCancel);

	bool RefreshCurrent();

private:
	AssetLoadResult TryLoadCore(QString fileName);

signals:
	void AssetAdded(int index);
	void AboutToCloseAsset(int index);
	void AboutToRemoveAsset(int index);
	void AssetRemoved(int index);
	void ActiveAssetChanged(Asset* asset);

private slots:
	void OnAssetAdded(int index);
	void OnAssetFileNameChanged(Asset* asset);

private:
	AssetManager* const _application;
	const std::shared_ptr<spdlog::logger> _logger;

	const std::unique_ptr<ObservableAssetList> _assetList;
	QPointer<Asset> _currentAsset;
};
