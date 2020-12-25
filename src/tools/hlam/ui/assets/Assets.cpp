#include <algorithm>
#include <stdexcept>

#include "assets/AssetIO.hpp"
#include "ui/assets/Assets.hpp"

namespace ui::assets
{
std::vector<AssetProvider*> AssetProviderRegistry::GetAssetProviders() const
{
	std::vector<AssetProvider*> providers;

	providers.reserve(_providers.size());

	std::transform(_providers.begin(), _providers.end(), std::back_inserter(providers), [](const auto& provider)
		{
			return provider.second.get();
		});

	return providers;
}

void AssetProviderRegistry::AddProvider(std::unique_ptr<AssetProvider>&& provider)
{
	if (_providers.find(provider->GetAssetType()) != _providers.end())
	{
		throw std::invalid_argument("Only one provider can be registered for a particular asset type");
	}

	_providers.emplace(provider->GetAssetType(), std::move(provider));
}

std::unique_ptr<Asset> AssetProviderRegistry::Load(EditorContext* editorContext, const QString& fileName) const
{
	for (const auto& provider : _providers)
	{
		if (provider.second->CanLoad(fileName))
		{
			return provider.second->Load(editorContext, fileName);
		}
	}

	throw ::assets::AssetException("File type not supported");
}
}
