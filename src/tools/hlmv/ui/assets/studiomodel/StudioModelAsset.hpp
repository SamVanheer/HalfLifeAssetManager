#pragma once

#include <memory>

#include "engine/shared/studiomodel/CStudioModel.h"

#include "ui/assets/Assets.hpp"

namespace ui::assets::studiomodel
{
class StudioModelAssetProvider;
class StudioModelContext;

class StudioModelAsset final : public IAsset
{
public:
	StudioModelAsset(EditorContext* editorContext, const StudioModelAssetProvider* provider, std::unique_ptr<studiomdl::CStudioModel>&& studioModel);

	~StudioModelAsset();
	StudioModelAsset(const StudioModelAsset&) = delete;
	StudioModelAsset& operator=(const StudioModelAsset&) = delete;

	entt::id_type GetAssetType() const override { return entt::type_index<StudioModelAsset>::value(); }

	const IAssetProvider* GetProvider() const override;

	QWidget* CreateEditWidget(EditorContext* editorContext) override;

	void SetupFullscreenWidget(EditorContext* editorContext, FullscreenWidget* fullscreenWidget) override;

	void Save(const std::string& fileName) override;

	StudioModelContext* GetContext() const { return _context.get(); }

	studiomdl::CStudioModel* GetStudioModel() { return _studioModel.get(); }

private:
	const StudioModelAssetProvider* const _provider;
	const std::unique_ptr<studiomdl::CStudioModel> _studioModel;
	const std::unique_ptr<StudioModelContext> _context;
};

class StudioModelAssetProvider final : public IAssetProvider
{
public:
	entt::id_type GetAssetType() const override { return entt::type_index<StudioModelAsset>::value(); }

	bool CanLoad(const std::string& fileName) const override;

	std::unique_ptr<IAsset> Load(EditorContext* editorContext, const std::string& fileName) const override;

	void Save(const std::string& fileName, IAsset& asset) const override;

	void Save(const std::string& fileName, StudioModelAsset& asset) const;
};

inline const IAssetProvider* StudioModelAsset::GetProvider() const
{
	return _provider;
}
}
