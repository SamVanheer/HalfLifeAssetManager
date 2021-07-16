#pragma once

#include <cassert>
#include <memory>
#include <stack>
#include <vector>

#include <QObject>

#include "engine/shared/studiomodel/EditableStudioModel.hpp"

#include "graphics/Scene.hpp"

#include "ui/IInputSink.hpp"
#include "ui/assets/Assets.hpp"

namespace graphics
{
class TextureLoader;
}

namespace ui
{
namespace camera_operators
{
class CameraOperator;
class CameraOperators;
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

class StudioModelAssetProvider final : public AssetProvider
{
public:
	StudioModelAssetProvider(const std::shared_ptr<settings::StudioModelSettings>& studioModelSettings)
		: _studioModelSettings(studioModelSettings)
	{
	}

	~StudioModelAssetProvider();

	QString GetProviderName() const override;

	QStringList GetFileTypes() const override;

	QString GetPreferredFileType() const override;

	QMenu* CreateToolMenu(EditorContext* editorContext) override;

	bool CanLoad(const QString& fileName) const override;

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const override;

	settings::StudioModelSettings* GetStudioModelSettings() const { return _studioModelSettings.get(); }

private:
	const std::shared_ptr<settings::StudioModelSettings> _studioModelSettings;
};

class StudioModelAsset final : public Asset, public IInputSink
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

	void OnMouseEvent(QMouseEvent* event) override;

	void OnWheelEvent(QWheelEvent* event) override;

	EditorContext* GetEditorContext() { return _editorContext; }

	studiomdl::EditableStudioModel* GetEditableStudioModel() { return _editableStudioModel.get(); }

	graphics::TextureLoader* GetTextureLoader() { return _textureLoader.get(); }

	graphics::Scene* GetScene() { return _scene.get(); }

	IInputSink* GetInputSink() const { return _inputSinks.top(); }

	void PushInputSink(IInputSink* inputSink)
	{
		assert(inputSink);

		_inputSinks.push(inputSink);
	}

	void PopInputSink()
	{
		_inputSinks.pop();
	}

	camera_operators::CameraOperators* GetCameraOperators() const { return _cameraOperators; }

	void AddUndoCommand(QUndoCommand* command)
	{
		GetUndoStack()->push(command);
	}

	void EmitModelChanged(const ModelChangeEvent& event)
	{
		emit ModelChanged(event);
	}

signals:
	void Tick();

	void ModelChanged(const ModelChangeEvent& event);

private slots:
	void OnTick();

	void OnSceneWidgetMouseEvent(QMouseEvent* event);

	void OnSceneWidgetWheelEvent(QWheelEvent* event);

	void OnCameraChanged(camera_operators::CameraOperator* previous, camera_operators::CameraOperator* current);

	void UpdateColors();

	void OnFloorLengthChanged(int length);

	void OnPreviousCamera();
	void OnNextCamera();

	void OnCenterView();
	void OnSaveView();
	void OnRestoreView();

	void OnLoadGroundTexture();
	void OnUnloadGroundTexture();

	void OnLoadBackgroundTexture();
	void OnUnloadBackgroundTexture();

	void OnDumpModelInfo();

	void OnTakeScreenshot();

private:
	EditorContext* const _editorContext;
	const StudioModelAssetProvider* const _provider;
	const std::unique_ptr<studiomdl::EditableStudioModel> _editableStudioModel;
	const std::unique_ptr<graphics::TextureLoader> _textureLoader;
	const std::unique_ptr<graphics::Scene> _scene;

	std::stack<IInputSink*> _inputSinks;

	camera_operators::CameraOperators* _cameraOperators;

	camera_operators::CameraOperator* _firstPersonCamera;

	StudioModelEditWidget* _editWidget{};
};
}
}
