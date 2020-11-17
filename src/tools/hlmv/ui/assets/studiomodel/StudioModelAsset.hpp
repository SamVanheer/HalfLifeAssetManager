#pragma once

#include "ui/assets/Assets.hpp"

namespace ui::assets::studiomodel
{
class StudioModelAssetProvider;

class StudioModelAsset final : public IAsset
{
public:
	//TODO: pass in loaded asset
	StudioModelAsset(const StudioModelAssetProvider* provider)
		: _provider(provider)
	{
	}

	~StudioModelAsset() = default;
	StudioModelAsset(const StudioModelAsset&) = delete;
	StudioModelAsset& operator=(const StudioModelAsset&) = delete;

	entt::id_type GetAssetType() const override { return entt::type_index<StudioModelAsset>::value(); }

	const IAssetProvider* GetProvider() const override;

	QWidget* CreateEditWidget() override;

	void Save(const std::string& fileName) override;

private:
	const StudioModelAssetProvider* const _provider;
};

class StudioModelAssetProvider final : public IAssetProvider
{
public:
	entt::id_type GetAssetType() const override { return entt::type_index<StudioModelAsset>::value(); }

	bool CanLoad(const std::string& fileName) const override;

	std::unique_ptr<IAsset> Load(const std::string& fileName) const override;

	void Save(const std::string& fileName, IAsset& asset) const override;

	void Save(const std::string& fileName, StudioModelAsset& asset) const;
};

inline const IAssetProvider* StudioModelAsset::GetProvider() const
{
	return _provider;
}
}
