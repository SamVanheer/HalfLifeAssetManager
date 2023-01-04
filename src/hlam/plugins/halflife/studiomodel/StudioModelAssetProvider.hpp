#pragma once

#include <memory>

#include <QLoggingCategory>
#include <QPointer>

#include "assets/Assets.hpp"

#include "graphics/OpenGL.hpp"

#include "ui/StateSnapshot.hpp"

class QAction;
class StudioModelSettings;

namespace sprite
{
class ISpriteRenderer;
}

namespace studiomdl
{
class IStudioModelRenderer;
}

namespace studiomodel
{
class StudioModelAsset;
class StudioModelEditWidget;

inline const QString XashModelViewerFileNameKey{QStringLiteral("XashModelViewerFileName")};
inline const QString StudioModelExtension{QStringLiteral("mdl")};
inline const QString StudioModelPS2Extension{QStringLiteral("dol")};

Q_DECLARE_LOGGING_CATEGORY(HLAMStudioModel)

class StudioModelAssetProvider final : public AssetProvider
{
	Q_OBJECT

public:
	explicit StudioModelAssetProvider(const std::shared_ptr<StudioModelSettings>& studioModelSettings);
	~StudioModelAssetProvider();

	QString GetProviderName() const override { return "Studiomodel"; }

	QStringList GetFileTypes() const override { return {StudioModelExtension}; }

	QString GetPreferredFileType() const override { return StudioModelExtension; }

	ProviderFeatures GetFeatures() const override
	{
		return ProviderFeature::AssetLoading | ProviderFeature::AssetSaving;
	}

	void Initialize(EditorContext* editorContext) override;

	void Shutdown() override;

	QMenu* CreateToolMenu() override;

	void PopulateAssetMenu(QMenu* menu) override;

	bool CanLoad(const QString& fileName, FILE* file) const override;

	std::variant<std::unique_ptr<Asset>, AssetLoadInExternalProgram> Load(
		const QString& fileName, FILE* file) override;

	StudioModelSettings* GetStudioModelSettings() const { return _studioModelSettings.get(); }

	studiomdl::IStudioModelRenderer* GetStudioModelRenderer() const { return _studioModelRenderer.get(); }

	sprite::ISpriteRenderer* GetSpriteRenderer() const { return _spriteRenderer.get(); }

	bool AreEditControlsVisible() const;

	StudioModelEditWidget* GetEditWidget();

	StudioModelAsset* GetDummyAsset() const { return _dummyAsset.get(); }

	StudioModelAsset* GetCurrentAsset() const { return _currentAsset; }

	GLuint GetDefaultGroundTexture() const { return _defaultGroundTexture; }

signals:
	void Tick();

	void AssetChanged(StudioModelAsset* asset);

private slots:
	void OnTick();

	void OnActiveAssetChanged(Asset* asset);

private:
	const std::shared_ptr<StudioModelSettings> _studioModelSettings;
	std::unique_ptr<studiomdl::IStudioModelRenderer> _studioModelRenderer;
	std::unique_ptr<sprite::ISpriteRenderer> _spriteRenderer;
	std::unique_ptr<StudioModelAsset> _dummyAsset;

	QPointer<QAction> _editControlsVisibleAction;
	QPointer<QAction> _restoreViewAction;

	QPointer<StudioModelEditWidget> _editWidget;

	StudioModelAsset* _currentAsset{};

	StateSnapshot _cameraSnapshot;

	GLuint _defaultGroundTexture{0};
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

	bool CanLoad(const QString& fileName, FILE* file) const override
	{
		return _assetProvider->CanLoad(fileName, file);
	}

	std::variant<std::unique_ptr<Asset>, AssetLoadInExternalProgram> Load(
		const QString& fileName, FILE* file) override
	{
		return _assetProvider->Load(fileName, file);
	}

private:
	StudioModelAssetProvider* const _assetProvider;
};
}
