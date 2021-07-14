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
			return provider.get();
		});

	return providers;
}

void AssetProviderRegistry::AddProvider(std::unique_ptr<AssetProvider>&& provider)
{
	if (std::find_if(_providers.begin(), _providers.end(), [&](const auto& other)
		{
			return other->GetProviderName() == provider->GetProviderName();
		}) != _providers.end())
	{
		throw std::invalid_argument("Only one provider can be registered for a particular asset type");
	}

	_providers.push_back(std::move(provider));
}

std::unique_ptr<Asset> AssetProviderRegistry::Load(EditorContext* editorContext, const QString& fileName) const
{
	for (const auto& provider : _providers)
	{
		if (provider->CanLoad(fileName))
		{
			return provider->Load(editorContext, fileName);
		}
	}

	throw ::assets::AssetException("File type not supported");
}
}
