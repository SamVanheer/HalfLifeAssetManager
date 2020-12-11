#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <QWidget>

#include <entt/core/type_info.hpp>

namespace ui
{
class EditorContext;
class FullscreenWidget;

namespace assets
{
class IAssetProvider;

//TODO: add a way to access the underlying asset data, identifying the asset type (use EnTT)
class IAsset
{
public:
	virtual ~IAsset() = 0 {}

	virtual entt::id_type GetAssetType() const = 0;

	virtual const IAssetProvider* GetProvider() const = 0;

	/**
	*	@brief Creates a widget to view and edit this asset
	*	@param editorContext The editor context used to communicate with the owner
	*/
	virtual QWidget* CreateEditWidget(EditorContext* editorContext) = 0;

	virtual FullscreenWidget* CreateFullscreenWidget(EditorContext* editorContext) = 0;

	virtual void Save(const std::string& fileName) = 0;
};

/**
*	@brief Provides a means of loading and saving assets
*/
class IAssetProvider
{
public:
	virtual ~IAssetProvider() = 0 {}

	virtual entt::id_type GetAssetType() const = 0;

	virtual bool CanLoad(const std::string& fileName) const = 0;

	//TODO: pass a filesystem object to resolve additional file locations with
	virtual std::unique_ptr<IAsset> Load(EditorContext* editorContext, const std::string& fileName) const = 0;

	virtual void Save(const std::string& fileName, IAsset& asset) const = 0;
};

/**
*	@brief Stores the list of asset providers
*/
class IAssetProviderRegistry
{
public:
	virtual ~IAssetProviderRegistry() = 0 {}

	virtual void AddProvider(std::unique_ptr<IAssetProvider>&& provider) = 0;

	virtual std::unique_ptr<IAsset> Load(EditorContext* editorContext, const std::string& fileName) const = 0;
};

class AssetProviderRegistry final : public IAssetProviderRegistry
{
public:
	AssetProviderRegistry() = default;
	~AssetProviderRegistry() = default;
	AssetProviderRegistry(const AssetProviderRegistry&) = delete;
	AssetProviderRegistry& operator=(const AssetProviderRegistry&) = delete;

	void AddProvider(std::unique_ptr<IAssetProvider>&& provider) override;

	std::unique_ptr<IAsset> Load(EditorContext* editorContext, const std::string& fileName) const override;

private:
	std::unordered_map<entt::id_type, std::unique_ptr<IAssetProvider>> _providers;
};
}
}
