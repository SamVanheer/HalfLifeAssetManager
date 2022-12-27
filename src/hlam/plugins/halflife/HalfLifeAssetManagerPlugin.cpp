#include "application/ApplicationBuilder.hpp"
#include "plugins/halflife/HalfLifeAssetManagerPlugin.hpp"

#include "settings/ColorSettings.hpp"
#include "settings/StudioModelSettings.hpp"

#include "ui/assets/Assets.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"

#include "ui/options/OptionsPageRegistry.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"

using namespace studiomodel;

bool HalfLifeAssetManagerPlugin::Initialize(ApplicationBuilder& builder)
{
	builder.ColorSettings->Add(GroundColor, RGBA8888ToVector(216, 216, 175, 178));
	builder.ColorSettings->Add(BackgroundColor, RGB888ToVector(63, 127, 127));
	builder.ColorSettings->Add(CrosshairColor, RGB888ToVector(255, 0, 0));
	builder.ColorSettings->Add(SkyLightColor, RGB888ToVector(255, 255, 255));
	builder.ColorSettings->Add(WireframeColor, RGB888ToVector(255, 0, 0));
	builder.ColorSettings->Add(HitboxEdgeColor, RGBA8888ToVector(255, 0, 0, 128));
	builder.ColorSettings->Add(HitboxFaceColor, RGBA8888ToVector(128, 0, 0, 0));

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
