#include <stdexcept>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelEditWidget.hpp"

namespace ui::assets::studiomodel
{
QWidget* StudioModelAsset::CreateEditWidget()
{
	return new StudioModelEditWidget(this);
}

void StudioModelAsset::Save(const std::string& fileName)
{
	_provider->Save(fileName, *this);
}

bool StudioModelAssetProvider::CanLoad(const std::string& fileName) const
{
	//TODO:
	return true;
}

std::unique_ptr<IAsset> StudioModelAssetProvider::Load(const std::string& fileName) const
{
	//TODO: load model
	return std::make_unique<StudioModelAsset>(this);
}

void StudioModelAssetProvider::Save(const std::string& fileName, IAsset& asset) const
{
	//TODO:
	if (asset.GetAssetType() == GetAssetType())
	{
		Save(fileName, static_cast<StudioModelAsset&>(asset));
	}
	else
	{
		//TODO: maybe allow conversion from other asset types to this one, otherwise remove this method from the provider API
		throw std::runtime_error("Cannot convert asset type to studiomodel");
	}
}

void StudioModelAssetProvider::Save(const std::string& fileName, StudioModelAsset& asset) const
{
	//TODO:
}
}
