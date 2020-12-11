#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <QObject>
#include <QString>
#include <QWidget>

#include <entt/core/type_info.hpp>

namespace ui
{
class EditorContext;
class FullscreenWidget;

namespace assets
{
class AssetDataChangeEvent;
class IAssetProvider;

class Asset : public QObject
{
	Q_OBJECT

protected:
	Asset(QString&& fileName)
		: _fileName(std::move(fileName))
	{
	}

public:
	virtual ~Asset() = 0 {}

	virtual entt::id_type GetAssetType() const = 0;

	virtual const IAssetProvider* GetProvider() const = 0;

	QString GetFileName() const { return _fileName; }

	void SetFileName(QString&& fileName)
	{
		if (_fileName != fileName)
		{
			_fileName = std::move(fileName);

			emit FileNameChanged(fileName);
		}
	}

	bool HasUnsavedChanges() const { return _hasUnsavedChanges; }

	void SetHasUnsavedChanges(bool value)
	{
		if (_hasUnsavedChanges != value)
		{
			_hasUnsavedChanges = value;
			emit HasUnsavedChangesChanged(_hasUnsavedChanges);
		}
	}

	/**
	*	@brief Creates a widget to view and edit this asset
	*	@param editorContext The editor context used to communicate with the owner
	*/
	virtual QWidget* CreateEditWidget(EditorContext* editorContext) = 0;

	virtual void SetupFullscreenWidget(EditorContext* editorContext, FullscreenWidget* fullscreenWidget) = 0;

	virtual void Save(const QString& fileName) = 0;

	void EmitAssetDataChanged(const AssetDataChangeEvent& event)
	{
		SetHasUnsavedChanges(true);
		emit AssetDataChanged(event);
	}

signals:
	void FileNameChanged(const QString& fileName);

	void HasUnsavedChangesChanged(bool value);

	/**
	*	@brief Emitted when any data changes in the asset
	*/
	void AssetDataChanged(const AssetDataChangeEvent& event);

protected:
	QString _fileName;
	bool _hasUnsavedChanges{false};
};

/**
*	@brief Provides a means of loading and saving assets
*/
class IAssetProvider
{
public:
	virtual ~IAssetProvider() = 0 {}

	virtual entt::id_type GetAssetType() const = 0;

	virtual bool CanLoad(const QString& fileName) const = 0;

	//TODO: pass a filesystem object to resolve additional file locations with
	virtual std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const = 0;

	virtual void Save(const QString& fileName, Asset& asset) const = 0;
};

/**
*	@brief Stores the list of asset providers
*/
class IAssetProviderRegistry
{
public:
	virtual ~IAssetProviderRegistry() = 0 {}

	virtual void AddProvider(std::unique_ptr<IAssetProvider>&& provider) = 0;

	virtual std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const = 0;
};

class AssetProviderRegistry final : public IAssetProviderRegistry
{
public:
	AssetProviderRegistry() = default;
	~AssetProviderRegistry() = default;
	AssetProviderRegistry(const AssetProviderRegistry&) = delete;
	AssetProviderRegistry& operator=(const AssetProviderRegistry&) = delete;

	void AddProvider(std::unique_ptr<IAssetProvider>&& provider) override;

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const override;

private:
	std::unordered_map<entt::id_type, std::unique_ptr<IAssetProvider>> _providers;
};
}
}
