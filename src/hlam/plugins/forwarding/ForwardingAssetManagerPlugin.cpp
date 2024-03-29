#include <cstdio>
#include <cstring>

#include "application/AssetIO.hpp"
#include "application/AssetManager.hpp"
#include "application/Assets.hpp"

#include "formats/studiomodel/StudioModelFileFormat.hpp"

#include "plugins/forwarding/ForwardingAssetManagerPlugin.hpp"

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

	AssetLoadData Load(const QString& fileName, FILE* file) override
	{
		return AssetLoadInExternalProgram{.ExternalProgramKey = Quake1ModelViewerFileNameKey};
	}
};

static const QString Source1ModelViewerFileNameKey{QStringLiteral("Source1ModelViewerFileName")};
const QString Source1ModelExtension{QStringLiteral("mdl")};

constexpr int Source1StudioVersionMin = 44;
constexpr int Source1StudioVersionMax = 48;

class Source1StudioModelAssetProvider final : public AssetProvider
{
public:
	explicit Source1StudioModelAssetProvider(AssetManager* application)
		: AssetProvider(application)
	{
	}

	QString GetProviderName() const override { return "Source 1 Studio model"; }

	QStringList GetFileTypes() const override { return {Source1ModelExtension}; }

	QString GetPreferredFileType() const override { return Source1ModelExtension; }

	ProviderFeatures GetFeatures() const override { return ProviderFeature::AssetLoading; }

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

	AssetLoadData Load(const QString& fileName, FILE* file) override
	{
		return AssetLoadInExternalProgram{.ExternalProgramKey = Source1ModelViewerFileNameKey};
	}
};

static const QString NexonModelViewerFileNameKey{QStringLiteral("NexonModelViewerFileName")};
const QString NexonModelExtension{QStringLiteral("mdl")};

constexpr int NexonStudioVersion = 11;

class NexonStudioModelAssetProvider final : public AssetProvider
{
public:
	explicit NexonStudioModelAssetProvider(AssetManager* application)
		: AssetProvider(application)
	{
	}

	QString GetProviderName() const override { return "Counter-Strike Nexon Studio model"; }

	QStringList GetFileTypes() const override { return {NexonModelExtension}; }

	QString GetPreferredFileType() const override { return NexonModelExtension; }

	ProviderFeatures GetFeatures() const override { return ProviderFeature::AssetLoading; }

	bool CanLoad(const QString& fileName, FILE* file) const override
	{
		int id;

		if (fread(&id, sizeof(id), 1, file) == 1
			&& strncmp(reinterpret_cast<const char*>(&id), STUDIOMDL_HDR_ID, 4) == 0)
		{
			int version;

			if (fread(&version, sizeof(version), 1, file) == 1
				&& NexonStudioVersion == version)
			{
				return true;
			}
		}

		return false;
	}

	AssetLoadData Load(const QString& fileName, FILE* file) override
	{
		return AssetLoadInExternalProgram{.ExternalProgramKey = NexonModelViewerFileNameKey};
	}
};

bool ForwardingAssetManagerPlugin::Initialize(AssetManager* application)
{
	const auto externalPrograms = application->GetApplicationSettings()->GetExternalPrograms();
	externalPrograms->AddProgram(Quake1ModelViewerFileNameKey, "Quake 1 Model Viewer");
	externalPrograms->AddProgram(Source1ModelViewerFileNameKey, "Source 1 Model Viewer");
	externalPrograms->AddProgram(NexonModelViewerFileNameKey, "Counter-Strike Nexon Model Viewer");

	application->GetAssetProviderRegistry()->AddProvider(std::make_unique<Quake1AliasModelAssetProvider>(application));
	application->GetAssetProviderRegistry()->AddProvider(std::make_unique<Source1StudioModelAssetProvider>(application));
	application->GetAssetProviderRegistry()->AddProvider(std::make_unique<NexonStudioModelAssetProvider>(application));

	return true;
}
