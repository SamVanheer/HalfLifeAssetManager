#pragma once

#include <cassert>
#include <memory>
#include <stack>
#include <vector>

#include <QLoggingCategory>
#include <QObject>

#include "engine/shared/studiomodel/EditableStudioModel.hpp"

#include "graphics/Scene.hpp"

#include "ui/IInputSink.hpp"
#include "ui/assets/Assets.hpp"

#include "utility/mathlib.hpp"

namespace graphics
{
class TextureLoader;
}

namespace ui
{
class StateSnapshot;

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

	void TryRefresh() override;

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

private:
	void SaveEntityToSnapshot(StateSnapshot* snapshot);
	void LoadEntityFromSnapshot(StateSnapshot* snapshot);

signals:
	void Tick();

	void ModelChanged(const ModelChangeEvent& event);

	void SaveSnapshot(StateSnapshot* snapshot);

	void LoadSnapshot(StateSnapshot* snapshot);

private slots:
	void OnTick();

	void OnSceneWidgetMouseEvent(QMouseEvent* event);

	void OnSceneWidgetWheelEvent(QWheelEvent* event);

	void OnCameraChanged(camera_operators::CameraOperator* previous, camera_operators::CameraOperator* current);

	void UpdateColors();

	void OnFloorLengthChanged(int length);

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
	const std::unique_ptr<graphics::Scene> _scene;

	std::stack<IInputSink*> _inputSinks;

	camera_operators::CameraOperators* _cameraOperators;

	camera_operators::CameraOperator* _firstPersonCamera;

	StudioModelEditWidget* _editWidget{};
};
}
}
