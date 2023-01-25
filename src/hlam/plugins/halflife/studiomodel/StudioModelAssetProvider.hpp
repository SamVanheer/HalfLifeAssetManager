#pragma once

#include <memory>

#include <QLoggingCategory>
#include <QPointer>

#include <spdlog/logger.h>

#include "application/Assets.hpp"

#include "graphics/OpenGL.hpp"

#include "ui/StateSnapshot.hpp"

class ApplicationSettings;
class CameraOperators;
class QAction;
class QMenu;
class SceneCameraOperator;
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

inline const QString CrowbarFileNameKey{QStringLiteral("CrowbarFileName")};
inline const QString XashModelViewerFileNameKey{QStringLiteral("XashModelViewerFileName")};
inline const QString StudioModelExtension{QStringLiteral("mdl")};
inline const QString StudioModelPS2Extension{QStringLiteral("dol")};

class StudioModelAssetProvider final : public AssetProvider
{
	Q_OBJECT

public:
	explicit StudioModelAssetProvider(AssetManager* application,
		const std::shared_ptr<StudioModelSettings>& studioModelSettings);
	~StudioModelAssetProvider();

	QString GetProviderName() const override { return "StudioModel"; }

	QStringList GetFileTypes() const override { return {StudioModelExtension}; }

	QString GetPreferredFileType() const override { return StudioModelExtension; }

	ProviderFeatures GetFeatures() const override
	{
		return ProviderFeature::AssetLoading | ProviderFeature::AssetSaving;
	}

	void Shutdown() override;

	void Tick() override;

	QMenu* CreateToolMenu() override;

	void PopulateAssetMenu(QMenu* menu) override;

	bool CanLoad(const QString& fileName, FILE* file) const override;

	AssetLoadData Load(const QString& fileName, FILE* file) override;

	bool IsCandidateForLoading(const QString& fileName, FILE* file) const override;

	StudioModelSettings* GetStudioModelSettings() const { return _studioModelSettings.get(); }

	studiomdl::IStudioModelRenderer* GetStudioModelRenderer() const { return _studioModelRenderer.get(); }

	sprite::ISpriteRenderer* GetSpriteRenderer() const { return _spriteRenderer.get(); }

	bool AreEditControlsVisible() const;

	StudioModelEditWidget* GetEditWidget();

	CameraOperators* GetCameraOperators() const { return _cameraOperators; }

	SceneCameraOperator* GetArcBallCameraOperator() const { return _arcBallCamera; }

	SceneCameraOperator* GetFirstPersonCameraOperator() const { return _firstPersonCamera; }

	StudioModelAsset* GetDummyAsset() const { return _dummyAsset.get(); }

	StudioModelAsset* GetCurrentAsset() const { return _currentAsset; }

	GLuint GetDefaultGroundTexture() const { return _defaultGroundTexture; }

	bool CameraIsFirstPerson() const;

signals:
	void AssetChanged(StudioModelAsset* asset);

private slots:
	void UpdateSettingsState();

	void OnActiveAssetChanged(Asset* asset);

	void OnDumpModelInfo();

	void OnTakeScreenshot();

private:
	const std::shared_ptr<spdlog::logger> _logger;
	const std::shared_ptr<StudioModelSettings> _studioModelSettings;
	const std::unique_ptr<studiomdl::IStudioModelRenderer> _studioModelRenderer;
	const std::unique_ptr<sprite::ISpriteRenderer> _spriteRenderer;
	const std::unique_ptr<StudioModelAsset> _dummyAsset;

	QPointer<QMenu> _toolMenu;
	QPointer<QAction> _launchCrowbarAction;
	QPointer<QAction> _editControlsVisibleAction;
	QPointer<QAction> _restoreViewAction;

	QPointer<StudioModelEditWidget> _editWidget;

	CameraOperators* const _cameraOperators;

	SceneCameraOperator* const _arcBallCamera;
	SceneCameraOperator* const _firstPersonCamera;

	StateSnapshot _cameraSnapshot;

	StudioModelAsset* _currentAsset{};

	GLuint _defaultGroundTexture{0};
};

/**
*	@brief Load-only provider that handles the loading of PS2 dol files
*/
class StudioModelDolImportProvider final : public AssetProvider
{
public:
	explicit StudioModelDolImportProvider(AssetManager* application, StudioModelAssetProvider* assetProvider)
		: AssetProvider(application)
		, _assetProvider(assetProvider)
	{
	}

	~StudioModelDolImportProvider() = default;

	QString GetProviderName() const override { return "StudioModel PS2"; }

	QStringList GetFileTypes() const override { return {StudioModelPS2Extension}; }

	QString GetPreferredFileType() const override { return StudioModelPS2Extension; }

	ProviderFeatures GetFeatures() const override { return ProviderFeature::AssetLoading; }

	bool CanLoad(const QString& fileName, FILE* file) const override
	{
		return _assetProvider->CanLoad(fileName, file);
	}

	AssetLoadData Load(const QString& fileName, FILE* file) override
	{
		return _assetProvider->Load(fileName, file);
	}

	bool IsCandidateForLoading(const QString& fileName, FILE* file) const override
	{
		return _assetProvider->IsCandidateForLoading(fileName, file);
	}

private:
	StudioModelAssetProvider* const _assetProvider;
};
}
