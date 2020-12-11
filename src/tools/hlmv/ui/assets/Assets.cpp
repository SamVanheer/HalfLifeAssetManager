#include <stdexcept>

#include "ui/assets/Assets.hpp"

namespace ui::assets
{
void AssetProviderRegistry::AddProvider(std::unique_ptr<IAssetProvider>&& provider)
{
	if (_providers.find(provider->GetAssetType()) != _providers.end())
	{
		throw std::invalid_argument("Only one provider can be registered for a particular asset type");
	}

	_providers.emplace(provider->GetAssetType(), std::move(provider));
}

std::unique_ptr<IAsset> AssetProviderRegistry::Load(EditorContext* editorContext, const QString& fileName) const
{
	for (const auto& provider : _providers)
	{
		if (provider.second->CanLoad(fileName))
		{
			return provider.second->Load(editorContext, fileName);
		}
	}

	//TODO: throw exception to indicate no supported provider

	return {};
}
}
