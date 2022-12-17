#include "application/ApplicationBuilder.hpp"
#include "plugins/halflife/HalfLifeAssetManagerPlugin.hpp"

#include "ui/assets/Assets.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"

#include "ui/options/OptionsPageRegistry.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"

#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/StudioModelSettings.hpp"

using namespace studiomodel;

bool HalfLifeAssetManagerPlugin::Initialize(ApplicationBuilder& builder)
{
	//TODO: this needs to be simplified
	const auto addColor = [&](const ColorInfo& color)
	{
		builder.ColorSettings->Add(color.Name, color.DefaultColor);
	};

	addColor(GroundColor);
	addColor(BackgroundColor);
	addColor(CrosshairColor);
	addColor(LightColor);
	addColor(WireframeColor);

	const auto studioModelSettings{std::make_shared<StudioModelSettings>()};

	// TODO: needs to be moved later on
	studioModelSettings->LoadSettings(*builder.Settings);

	auto studioModelAssetProvider = std::make_unique<StudioModelAssetProvider>(studioModelSettings);
	auto studioModelImportProvider = std::make_unique<StudioModelDolImportProvider>(studioModelAssetProvider.get());

	builder.AssetProviderRegistry->AddProvider(std::move(studioModelAssetProvider));
	builder.AssetProviderRegistry->AddProvider(std::move(studioModelImportProvider));

	builder.OptionsPageRegistry->AddPage(std::make_unique<OptionsPageStudioModel>(studioModelSettings));

	return true;
}

void HalfLifeAssetManagerPlugin::Shutdown()
{
}

void HalfLifeAssetManagerPlugin::LoadSettings(QSettings& settings)
{
}

void HalfLifeAssetManagerPlugin::SaveSettings(QSettings& settings)
{
}
