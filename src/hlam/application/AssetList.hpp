#pragma once

#include <memory>
#include <vector>

#include <QObject>
#include <QPointer>

class Asset;
class AssetManager;

enum class AssetLoadResult
{
	Success = 0,
	Failed,
	Cancelled
};

class AssetList final : public QObject
{
	Q_OBJECT

public:
	explicit AssetList(AssetManager* application);
	~AssetList() override;

	std::size_t Count() const { return _assets.size(); }

	int IndexOf(const Asset* asset) const;

	Asset* Get(std::size_t index) const { return _assets[index].get(); }

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
	void OnAssetFileNameChanged(Asset* asset);

private:
	AssetManager* const _application;

	std::vector<std::unique_ptr<Asset>> _assets;
	QPointer<Asset> _currentAsset;
};
