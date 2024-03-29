#include "application/AssetManager.hpp"
#include "application/Assets.hpp"

#include "plugins/halflife/HalfLifeAssetManagerPlugin.hpp"
#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/StudioModelColors.hpp"

#include "plugins/halflife/studiomodel/options/OptionsPageStudioModel.hpp"
#include "plugins/halflife/studiomodel/settings/StudioModelSettings.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/ColorSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"

#include "ui/options/OptionsPageRegistry.hpp"

using namespace studiomodel;

bool HalfLifeAssetManagerPlugin::Initialize(AssetManager* application)
{
	auto settings = application->GetApplicationSettings();
	settings->GetExternalPrograms()->AddProgram(CrowbarFileNameKey, "Crowbar (De)compiler");
	settings->GetExternalPrograms()->AddProgram(XashModelViewerFileNameKey, "Xash Model Viewer");

	auto colorSettings = settings->GetColorSettings();

	colorSettings->Add(GroundColor, RGBA8888ToVector(216, 216, 175, 178));
	colorSettings->Add(BackgroundColor, RGB888ToVector(63, 127, 127));
	colorSettings->Add(CrosshairColor, RGB888ToVector(255, 0, 0));
	colorSettings->Add(SkyLightColor, RGB888ToVector(255, 255, 255));
	colorSettings->Add(WireframeColor, RGB888ToVector(255, 0, 0));
	colorSettings->Add(HitboxEdgeColor, RGBA8888ToVector(255, 0, 0, 128));
	colorSettings->Add(HitboxFaceColor, RGBA8888ToVector(128, 0, 0, 0));
	colorSettings->Add(SingleHitboxEdgeColor, RGBA8888ToVector(0, 255, 0, 128));
	colorSettings->Add(SingleHitboxFaceColor, RGBA8888ToVector(0, 128, 0, 128));

	const auto studioModelSettings{std::make_shared<StudioModelSettings>(settings->GetSettings())};

	settings->RegisterSettings(studioModelSettings.get());

	auto studioModelAssetProvider = std::make_unique<StudioModelAssetProvider>(application, studioModelSettings);
	auto studioModelImportProvider = std::make_unique<StudioModelDolImportProvider>(
		application, studioModelAssetProvider.get());

	application->GetAssetProviderRegistry()->AddProvider(std::move(studioModelAssetProvider));
	application->GetAssetProviderRegistry()->AddProvider(std::move(studioModelImportProvider));

	application->GetOptionsPageRegistry()->AddPage(std::make_unique<OptionsPageStudioModel>(studioModelSettings));

	return true;
}

void HalfLifeAssetManagerPlugin::Shutdown()
{
}
