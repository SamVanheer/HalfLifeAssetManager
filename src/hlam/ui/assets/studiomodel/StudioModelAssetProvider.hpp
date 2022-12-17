#pragma once

#include <memory>

#include <QLoggingCategory>

#include "ui/assets/Assets.hpp"

class StudioModelSettings;

namespace studiomodel
{
inline const QString StudioModelExtension{QStringLiteral("mdl")};
inline const QString StudioModelPS2Extension{QStringLiteral("dol")};

Q_DECLARE_LOGGING_CATEGORY(HLAMStudioModel)

class StudioModelAssetProvider final : public AssetProvider
{
public:
	StudioModelAssetProvider(const std::shared_ptr<StudioModelSettings>& studioModelSettings)
		: _studioModelSettings(studioModelSettings)
	{
	}

	~StudioModelAssetProvider();

	QString GetProviderName() const override { return "Studiomodel"; }

	QStringList GetFileTypes() const override { return {StudioModelExtension}; }

	QString GetPreferredFileType() const override { return StudioModelExtension; }

	ProviderFeatures GetFeatures() const override
	{
		return ProviderFeature::AssetLoading | ProviderFeature::AssetSaving;
	}

	QMenu* CreateToolMenu(EditorContext* editorContext) override;

	bool CanLoad(const QString& fileName, FILE* file) const override;

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName, FILE* file) const override;

	StudioModelSettings* GetStudioModelSettings() const { return _studioModelSettings.get(); }

private:
	const std::shared_ptr<StudioModelSettings> _studioModelSettings;
};

/**
*	@brief Load-only provider that handles the loading of PS2 dol files
*/
class StudioModelDolImportProvider final : public AssetProvider
{
public:
	StudioModelDolImportProvider(StudioModelAssetProvider* assetProvider)
		: _assetProvider(assetProvider)
	{
	}

	~StudioModelDolImportProvider() = default;

	QString GetProviderName() const override { return "Studiomodel PS2"; }

	QStringList GetFileTypes() const override { return {StudioModelPS2Extension}; }

	QString GetPreferredFileType() const override { return StudioModelPS2Extension; }

	ProviderFeatures GetFeatures() const override { return ProviderFeature::AssetLoading; }

	QMenu* CreateToolMenu(EditorContext* editorContext) override { return nullptr; }

	bool CanLoad(const QString& fileName, FILE* file) const override
	{
		return _assetProvider->CanLoad(fileName, file);
	}

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName, FILE* file) const override
	{
		return _assetProvider->Load(editorContext, fileName, file);
	}

private:
	StudioModelAssetProvider* const _assetProvider;
};
}
