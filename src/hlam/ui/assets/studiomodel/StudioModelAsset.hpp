#pragma once

#include <memory>
#include <vector>

#include "formats/studiomodel/EditableStudioModel.hpp"

#include "ui/assets/Assets.hpp"
#include "ui/assets/studiomodel/StudioModelAssetProvider.hpp"

#include "utility/mathlib.hpp"

class BackgroundEntity;
class CameraOperators;
class EntityContext;
class GroundEntity;
class HLMVStudioModelEntity;
class SceneCameraOperator;
class StateSnapshot;
class TextureCameraOperator;
class TextureEntity;

namespace graphics
{
class IGraphicsContext;
class Scene;
class TextureLoader;
}

namespace studiomdl
{
class IStudioModelRenderer;
}

namespace studiomodel
{
class StudioModelData;
class StudioModelEditWidget;

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

	StudioModelData* GetModelData() { return _modelData; }

	graphics::IGraphicsContext* GetGraphicsContext();

	graphics::TextureLoader* GetTextureLoader();

	studiomdl::IStudioModelRenderer* GetStudioModelRenderer() const;

	const std::vector<graphics::Scene*>& GetScenes() { return _scenes; }

	void SetCurrentScene(graphics::Scene* scene);

	graphics::Scene* GetScene() { return _scene.get(); }

	CameraOperators* GetCameraOperators() const { return _cameraOperators; }

	void AddUndoCommand(QUndoCommand* command)
	{
		GetUndoStack()->push(command);
	}

	HLMVStudioModelEntity* GetEntity() { return _modelEntity.get(); }

	BackgroundEntity* GetBackgroundEntity() { return _backgroundEntity.get(); }

	GroundEntity* GetGroundEntity() { return _groundEntity.get(); }

	graphics::Scene* GetTextureScene() { return _textureScene.get(); }

	TextureEntity* GetTextureEntity() { return _textureEntity.get(); }

	TextureCameraOperator* GetTextureCameraOperator() { return _textureCameraOperator.get(); }

	Pose GetPose() const { return _pose; }

private:
	void CreateMainScene();
	void CreateTextureScene();

	void SaveEntityToSnapshot(StateSnapshot* snapshot);
	void LoadEntityFromSnapshot(StateSnapshot* snapshot);

signals:
	void Tick();

	void AssetChanged(StudioModelAsset* asset);

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

	void OnResizeTexturesToPowerOf2Changed();

	void OnTextureFiltersChanged();

	void OnSceneIndexChanged(int index);

	void OnSceneWidgetMouseEvent(QMouseEvent* event);

	void OnSceneWidgetWheelEvent(QWheelEvent* event);

	void OnCameraChanged(SceneCameraOperator* previous, SceneCameraOperator* current);

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
	StudioModelData* _modelData;
	const std::unique_ptr<EntityContext> _entityContext;

	std::vector<graphics::Scene*> _scenes;

	graphics::Scene* _currentScene{};

	std::unique_ptr<graphics::Scene> _scene;

	CameraOperators* _cameraOperators;

	SceneCameraOperator* _firstPersonCamera;

	StudioModelEditWidget* _editWidget{};

	std::shared_ptr<HLMVStudioModelEntity> _modelEntity;
	std::shared_ptr<BackgroundEntity> _backgroundEntity;
	std::shared_ptr<GroundEntity> _groundEntity;

	std::unique_ptr<graphics::Scene> _textureScene;

	std::shared_ptr<TextureEntity> _textureEntity;

	std::unique_ptr<TextureCameraOperator> _textureCameraOperator;

	//TODO: this is temporarily put here, but needs to be put somewhere else eventually
	Pose _pose = Pose::Sequences;
};
}
