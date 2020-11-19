#pragma once

#include <memory>

#include "engine/shared/studiomodel/CStudioModel.h"

#include "ui/assets/Assets.hpp"

namespace ui::assets::studiomodel
{
class StudioModelAssetProvider;

class StudioModelAsset final : public IAsset
{
public:
	//TODO: pass in loaded asset
	StudioModelAsset(const StudioModelAssetProvider* provider, std::unique_ptr<studiomdl::CStudioModel>&& studioModel)
		: _provider(provider)
		, _studioModel(std::move(studioModel))
	{
	}

	~StudioModelAsset() = default;
	StudioModelAsset(const StudioModelAsset&) = delete;
	StudioModelAsset& operator=(const StudioModelAsset&) = delete;

	entt::id_type GetAssetType() const override { return entt::type_index<StudioModelAsset>::value(); }

	const IAssetProvider* GetProvider() const override;

	QWidget* CreateEditWidget() override;

	void Save(const std::string& fileName) override;

	studiomdl::CStudioModel* GetStudioModel() { return _studioModel.get(); }

private:
	const StudioModelAssetProvider* const _provider;

	std::unique_ptr<studiomdl::CStudioModel> _studioModel;
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
