#pragma once

#include <cassert>
#include <memory>
#include <stack>
#include <vector>

#include <QColor>
#include <QObject>

#include "engine/shared/studiomodel/CStudioModel.h"

#include "graphics/Scene.hpp"

#include "ui/IInputSink.hpp"
#include "ui/assets/Assets.hpp"

namespace ui
{
namespace camera_operators
{
class CameraOperator;
}

namespace settings
{
class StudioModelSettings;
}

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAssetProvider;
class StudioModelEditWidget;

class StudioModelAsset final : public Asset, public IInputSink
{
	Q_OBJECT

public:
	StudioModelAsset(QString&& fileName,
		EditorContext* editorContext, const StudioModelAssetProvider* provider, std::unique_ptr<studiomdl::CStudioModel>&& studioModel);

	~StudioModelAsset();
	StudioModelAsset(const StudioModelAsset&) = delete;
	StudioModelAsset& operator=(const StudioModelAsset&) = delete;

	entt::id_type GetAssetType() const override { return entt::type_index<StudioModelAsset>::value(); }

	const IAssetProvider* GetProvider() const override;

	void PopulateAssetMenu(QMenu* menu) override;

	QWidget* GetEditWidget() override;

	void SetupFullscreenWidget(FullscreenWidget* fullscreenWidget) override;

	void Save(const QString& fileName) override;

	void OnMouseEvent(QMouseEvent* event) override;

	EditorContext* GetEditorContext() { return _editorContext; }

	studiomdl::CStudioModel* GetStudioModel() { return _studioModel.get(); }

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

	int GetCameraOperatorCount() const { return _cameraOperators.size(); }

	camera_operators::CameraOperator* GetCameraOperator(int index) const;

	void AddCameraOperator(std::unique_ptr<camera_operators::CameraOperator>&& cameraOperator);

	camera_operators::CameraOperator* GetCurrentCameraOperator() const { return _cameraOperator; }

	void SetCurrentCameraOperator(camera_operators::CameraOperator* cameraOperator);

	void AddUndoCommand(QUndoCommand* command)
	{
		GetUndoStack()->push(command);
	}

	void EmitModelChanged(const ModelChangeEvent& event)
	{
		emit ModelChanged(event);
	}

private:
	void ChangeCamera(bool next);

signals:
	void Tick();

	void Draw();

	void ModelChanged(const ModelChangeEvent& event);

	void CameraChanged(camera_operators::CameraOperator* cameraOperator);

private slots:
	void OnTick();

	void OnSceneWidgetMouseEvent(QMouseEvent* event);

	void SetBackgroundColor(QColor color)
	{
		_scene->SetBackgroundColor({color.redF(), color.greenF(), color.blueF()});
	}

	void OnFloorLengthChanged(int length);

	void OnPreviousCamera();
	void OnNextCamera();

	void OnLoadGroundTexture();
	void OnUnloadGroundTexture();

	void OnLoadBackgroundTexture();
	void OnUnloadBackgroundTexture();

	void OnDumpModelInfo();

	void OnTakeScreenshot();

private:
	EditorContext* const _editorContext;
	const StudioModelAssetProvider* const _provider;
	const std::unique_ptr<studiomdl::CStudioModel> _studioModel;
	const std::unique_ptr<graphics::Scene> _scene;

	std::stack<IInputSink*> _inputSinks;

	std::vector<std::unique_ptr<camera_operators::CameraOperator>> _cameraOperators;

	camera_operators::CameraOperator* _cameraOperator{};

	StudioModelEditWidget* _editWidget{};
};

class StudioModelAssetProvider final : public IAssetProvider
{
public:
	StudioModelAssetProvider(const std::shared_ptr<settings::StudioModelSettings>& studioModelSettings)
		: _settings(studioModelSettings)
	{
	}

	entt::id_type GetAssetType() const override { return entt::type_index<StudioModelAsset>::value(); }

	bool CanLoad(const QString& fileName) const override;

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const override;

	void Save(const QString& fileName, StudioModelAsset& asset) const;

	settings::StudioModelSettings* GetSettings() const { return _settings.get(); }

private:
	const std::shared_ptr<settings::StudioModelSettings> _settings;
};

inline const IAssetProvider* StudioModelAsset::GetProvider() const
{
	return _provider;
}
}
}
