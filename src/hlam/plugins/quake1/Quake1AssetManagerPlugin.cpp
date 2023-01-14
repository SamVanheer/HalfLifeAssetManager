#include <cstdio>
#include <cstring>

#include "application/AssetIO.hpp"
#include "application/AssetManager.hpp"
#include "application/Assets.hpp"
#include "plugins/quake1/Quake1AssetManagerPlugin.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"

static const QString Quake1ModelViewerFileNameKey{QStringLiteral("Quake1ModelViewerFileName")};
const QString AliasModelExtension{QStringLiteral("mdl")};

constexpr char AliasModelHeaderId[] = "IDPO";

class Quake1AliasModelAssetProvider final : public AssetProvider
{
public:
	explicit Quake1AliasModelAssetProvider(AssetManager* application)
		: AssetProvider(application)
	{
	}

	QString GetProviderName() const override { return "Quake 1 Alias model"; }

	QStringList GetFileTypes() const override { return {AliasModelExtension}; }

	QString GetPreferredFileType() const override { return AliasModelExtension; }

	ProviderFeatures GetFeatures() const override { return ProviderFeature::AssetLoading; }

	bool CanLoad(const QString& fileName, FILE* file) const override
	{
		int id;

		if (fread(&id, sizeof(id), 1, file) == 1)
		{
			if (strncmp(reinterpret_cast<const char*>(&id), AliasModelHeaderId, 4) == 0)
			{
				return true;
			}
		}

		return false;
	}

	std::variant<std::unique_ptr<Asset>, AssetLoadInExternalProgram> Load(
		const QString& fileName, FILE* file) override
	{
		const auto result = _application->TryLaunchExternalProgram(
			Quake1ModelViewerFileNameKey, QStringList(fileName),
			"This is a Quake 1 Alias model which requires it to be loaded in Quake 1 Model Viewer.");

		if (result != LaunchExternalProgramResult::Failed)
		{
			return AssetLoadInExternalProgram{.Loaded = result == LaunchExternalProgramResult::Success};
		}

		throw AssetException(std::string{"File \""} + fileName.toStdString()
			+ "\" is a Quake 1 Alias model and cannot be opened by this program."
			+ "\nSet the Quake 1 Model Viewer executable setting to open the model through that program instead.");
	}
};

bool Quake1AssetManagerPlugin::Initialize(AssetManager* application)
{
	application->GetApplicationSettings()->GetExternalPrograms()->AddProgram(
		Quake1ModelViewerFileNameKey, "Quake 1 Model Viewer");

	auto quake1AliasModelAssetProvider = std::make_unique<Quake1AliasModelAssetProvider>(application);
	application->GetAssetProviderRegistry()->AddProvider(std::move(quake1AliasModelAssetProvider));

	return true;
}
