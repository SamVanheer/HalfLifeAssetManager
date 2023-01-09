#include "application/ApplicationBuilder.hpp"
#include "application/Assets.hpp"

#include "plugins/halflife/HalfLifeAssetManagerPlugin.hpp"
#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/StudioModelColors.hpp"
#include "plugins/halflife/studiomodel/ui/compiler/StudioModelCompilerFrontEnd.hpp"
#include "plugins/halflife/studiomodel/ui/compiler/StudioModelDecompilerFrontEnd.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/ColorSettings.hpp"
#include "settings/StudioModelSettings.hpp"

#include "ui/options/OptionsPageRegistry.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"

using namespace studiomodel;

bool HalfLifeAssetManagerPlugin::Initialize(ApplicationBuilder& builder)
{
	builder.Settings->GetExternalPrograms()->AddProgram(StudiomdlCompilerFileNameKey, "Studiomdl compiler");
	builder.Settings->GetExternalPrograms()->AddProgram(StudiomdlDecompilerFileNameKey, "Studiomdl Decompiler");
	builder.Settings->GetExternalPrograms()->AddProgram(XashModelViewerFileNameKey, "Xash Model Viewer");

	auto colorSettings = builder.Settings->GetColorSettings();

	colorSettings->Add(GroundColor, RGBA8888ToVector(216, 216, 175, 178));
	colorSettings->Add(BackgroundColor, RGB888ToVector(63, 127, 127));
	colorSettings->Add(CrosshairColor, RGB888ToVector(255, 0, 0));
	colorSettings->Add(SkyLightColor, RGB888ToVector(255, 255, 255));
	colorSettings->Add(WireframeColor, RGB888ToVector(255, 0, 0));
	colorSettings->Add(HitboxEdgeColor, RGBA8888ToVector(255, 0, 0, 128));
	colorSettings->Add(HitboxFaceColor, RGBA8888ToVector(128, 0, 0, 0));

	const auto studioModelSettings{std::make_shared<StudioModelSettings>(builder.Settings->GetSettings())};

	QObject::connect(builder.Settings, &ApplicationSettings::SettingsSaved,
		studioModelSettings.get(), &StudioModelSettings::SaveSettings);

	// TODO: needs to be moved later on
	studioModelSettings->LoadSettings();

	auto studioModelAssetProvider = std::make_unique<StudioModelAssetProvider>(builder.Settings, studioModelSettings);
	auto studioModelImportProvider = std::make_unique<StudioModelDolImportProvider>(studioModelAssetProvider.get());

	builder.AssetProviders->AddProvider(std::move(studioModelAssetProvider));
	builder.AssetProviders->AddProvider(std::move(studioModelImportProvider));

	builder.OptionsPages->AddPage(std::make_unique<OptionsPageStudioModel>(studioModelSettings));

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
