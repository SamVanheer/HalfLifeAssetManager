#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <QLoggingCategory>
#include <QObject>
#include <QVector>

#include "formats/studiomodel/EditableStudioModel.hpp"

#include "ui/assets/Assets.hpp"

#include "utility/mathlib.hpp"

class BackgroundEntity;
class EntityContext;
class GroundEntity;
class HLMVStudioModelEntity;
class TextureEntity;

namespace graphics
{
class Scene;
class TextureLoader;
}

namespace soundsystem
{
class ISoundSystem;
}

namespace sprite
{
class ISpriteRenderer;
}

namespace studiomdl
{
class IStudioModelRenderer;
}

namespace ui
{
class StateSnapshot;

namespace camera_operators
{
class CameraOperators;
class SceneCameraOperator;
class TextureCameraOperator;
}

namespace settings
{
class StudioModelSettings;
}

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;
class StudioModelEditWidget;

inline const QString StudioModelExtension{QStringLiteral("mdl")};
inline const QString StudioModelPS2Extension{QStringLiteral("dol")};

Q_DECLARE_LOGGING_CATEGORY(HLAMStudioModel)

class StudioModelAssetProvider final : public AssetProvider
{
public:
	StudioModelAssetProvider(const std::shared_ptr<settings::StudioModelSettings>& studioModelSettings)
		: _studioModelSettings(studioModelSettings)
	{
	}

	~StudioModelAssetProvider();

	QString GetProviderName() const override { return "Studiomodel"; }

	QStringList GetFileTypes() const override { return {StudioModelExtension}; }

	QString GetPreferredFileType() const override { return StudioModelExtension; }

	ProviderFeatures GetFeatures() const override { return ProviderFeature::AssetLoading | ProviderFeature::AssetSaving; }

	QMenu* CreateToolMenu(EditorContext* editorContext) override;

	bool CanLoad(const QString& fileName, FILE* file) const override;

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName, FILE* file) const override;

	settings::StudioModelSettings* GetStudioModelSettings() const { return _studioModelSettings.get(); }

private:
	const std::shared_ptr<settings::StudioModelSettings> _studioModelSettings;
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

enum class Pose
{
	Sequences = 0,
	Skeleton
};

class StudioModelAsset final : public Asset
{
	Q_OBJECT

public:
	StudioModelAsset(QString&& fileName,
		EditorContext* editorContext, const StudioModelAssetProvider* provider,
		std::unique_ptr<studiomdl::EditableStudioModel>&& editableStudioModel);

	~StudioModelAsset();
	StudioModelAsset(const StudioModelAsset&) = delete;
	StudioModelAsset& operator=(const StudioModelAsset&) = delete;

	const StudioModelAssetProvider* GetProvider() const override { return _provider; }

	void PopulateAssetMenu(QMenu* menu) override;

	QWidget* GetEditWidget() override;

	void SetupFullscreenWidget(FullscreenWidget* fullscreenWidget) override;

	void Save() override;

	void TryRefresh() override;

	EditorContext* GetEditorContext() { return _editorContext; }

	studiomdl::EditableStudioModel* GetEditableStudioModel() { return _editableStudioModel.get(); }

	graphics::TextureLoader* GetTextureLoader() { return _textureLoader.get(); }

	studiomdl::IStudioModelRenderer* GetStudioModelRenderer() { return _studioModelRenderer.get(); }

	sprite::ISpriteRenderer* GetSpriteRenderer() { return _spriteRenderer.get(); }

	const QVector<graphics::Scene*>& GetScenes() { return _scenes; }

	void SetCurrentScene(graphics::Scene* scene);

	graphics::Scene* GetScene() { return _scene.get(); }

	camera_operators::CameraOperators* GetCameraOperators() const { return _cameraOperators; }

	void AddUndoCommand(QUndoCommand* command)
	{
		GetUndoStack()->push(command);
	}

	void EmitModelChanged(const ModelChangeEvent& event)
	{
		emit ModelChanged(event);
	}

	HLMVStudioModelEntity* GetEntity() { return _modelEntity.get(); }

	BackgroundEntity* GetBackgroundEntity() { return _backgroundEntity.get(); }

	GroundEntity* GetGroundEntity() { return _groundEntity.get(); }

	graphics::Scene* GetTextureScene() { return _textureScene.get(); }

	TextureEntity* GetTextureEntity() { return _textureEntity.get(); }

	camera_operators::TextureCameraOperator* GetTextureCameraOperator() { return _textureCameraOperator.get(); }

	Pose GetPose() const { return _pose; }

	soundsystem::ISoundSystem* GetSoundSystem();

private:
	void CreateMainScene();
	void CreateTextureScene();

	void SaveEntityToSnapshot(StateSnapshot* snapshot);
	void LoadEntityFromSnapshot(StateSnapshot* snapshot);

signals:
	void Tick();

	void ModelChanged(const ModelChangeEvent& event);

	void SaveSnapshot(StateSnapshot* snapshot);

	void LoadSnapshot(StateSnapshot* snapshot);

	void PoseChanged(Pose pose);

public slots:
	void SetPose(Pose pose)
	{
		_pose = pose;
		emit PoseChanged(pose);
	}

private slots:
	void OnTick();

	void OnSceneWidgetMouseEvent(QMouseEvent* event);

	void OnSceneWidgetWheelEvent(QWheelEvent* event);

	void OnCameraChanged(camera_operators::SceneCameraOperator* previous, camera_operators::SceneCameraOperator* current);

	void OnPreviousCamera();
	void OnNextCamera();

	void OnCenterView(Axis axis, bool positive);
	void OnSaveView();
	void OnRestoreView();

	void OnFlipNormals();

	void OnDumpModelInfo();

	void OnTakeScreenshot();

private:
	EditorContext* const _editorContext;
	const StudioModelAssetProvider* const _provider;
	std::unique_ptr<studiomdl::EditableStudioModel> _editableStudioModel;
	const std::unique_ptr<graphics::TextureLoader> _textureLoader;
	const std::unique_ptr<studiomdl::IStudioModelRenderer> _studioModelRenderer;
	const std::unique_ptr<sprite::ISpriteRenderer> _spriteRenderer;
	const std::unique_ptr<EntityContext> _entityContext;

	QVector<graphics::Scene*> _scenes;

	std::unique_ptr<graphics::Scene> _scene;

	camera_operators::CameraOperators* _cameraOperators;

	camera_operators::SceneCameraOperator* _firstPersonCamera;

	StudioModelEditWidget* _editWidget{};

	std::shared_ptr<HLMVStudioModelEntity> _modelEntity;
	std::shared_ptr<BackgroundEntity> _backgroundEntity;
	std::shared_ptr<GroundEntity> _groundEntity;

	std::unique_ptr<graphics::Scene> _textureScene;

	std::shared_ptr<TextureEntity> _textureEntity;

	std::unique_ptr<camera_operators::TextureCameraOperator> _textureCameraOperator;

	//TODO: this is temporarily put here, but needs to be put somewhere else eventually
	Pose _pose = Pose::Sequences;
};
}
}
