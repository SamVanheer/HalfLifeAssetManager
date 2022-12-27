#include <cstdio>
#include <cstring>

#include "application/ApplicationBuilder.hpp"
#include "assets/AssetIO.hpp"
#include "assets/Assets.hpp"
#include "formats/studiomodel/StudioModelFileFormat.hpp"
#include "plugins/source1/Source1AssetManagerPlugin.hpp"

#include "settings/ApplicationSettings.hpp"

#include "ui/EditorContext.hpp"

const QString Source1ModelExtension{QStringLiteral("mdl")};

constexpr int Source1StudioVersionMin = 44;
constexpr int Source1StudioVersionMax = 48;

class Source1StudioModelAssetProvider final : public AssetProvider
{
public:
	explicit Source1StudioModelAssetProvider(ApplicationSettings* applicationSettings)
		: _applicationSettings(applicationSettings)
	{
	}

	QString GetProviderName() const override { return "Source 1 Studio model"; }

	QStringList GetFileTypes() const override { return {Source1ModelExtension}; }

	QString GetPreferredFileType() const override { return Source1ModelExtension; }

	ProviderFeatures GetFeatures() const override { return ProviderFeature::AssetLoading; }

	QMenu* CreateToolMenu() override { return nullptr; }

	bool CanLoad(const QString& fileName, FILE* file) const override
	{
		int id;

		if (fread(&id, sizeof(id), 1, file) == 1
			&& strncmp(reinterpret_cast<const char*>(&id), STUDIOMDL_HDR_ID, 4) == 0)
		{
			int version;

			if (fread(&version, sizeof(version), 1, file) == 1
				&& Source1StudioVersionMin <= version && version <= Source1StudioVersionMax)
			{
				return true;
			}
		}

		return false;
	}

	std::variant<std::unique_ptr<Asset>, AssetLoadInExternalProgram> Load(
		const QString& fileName, FILE* file) override
	{
		const auto result = _editorContext->TryLaunchExternalProgram(
			_applicationSettings->GetSource1ModelViewerFileName(),
			QStringList(fileName),
			"This is a Source 1 Studio model which requires it to be loaded in Source 1 Half-Life Model Viewer.");

		if (result != LaunchExternalProgramResult::Failed)
		{
			return AssetLoadInExternalProgram{.Loaded = result == LaunchExternalProgramResult::Success};
		}

		throw AssetException(std::string{"File \""} + fileName.toStdString()
			+ "\" is a Source 1 Studio model and cannot be opened by this program."
			+ "\nSet the Source 1 Half-Life Model Viewer executable setting to open the model through that program instead.");
	}

private:
	ApplicationSettings* const _applicationSettings;
};

bool Source1AssetManagerPlugin::Initialize(ApplicationBuilder& builder)
{
	auto source1StudioModelAssetProvider = std::make_unique<Source1StudioModelAssetProvider>(builder.ApplicationSettings);
	builder.AssetProviderRegistry->AddProvider(std::move(source1StudioModelAssetProvider));
	return true;
}
