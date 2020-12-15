#pragma once

#include <cassert>
#include <memory>
#include <stack>

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
class StudioModelAssetProvider;

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

	QWidget* CreateEditWidget(EditorContext* editorContext) override;

	void SetupFullscreenWidget(EditorContext* editorContext, FullscreenWidget* fullscreenWidget) override;

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

signals:
	void Tick();

public slots:
	void OnSceneWidgetMouseEvent(QMouseEvent* event);

	void SetBackgroundColor(QColor color)
	{
		_scene->SetBackgroundColor({color.redF(), color.greenF(), color.blueF()});
	}

	void OnFloorLengthChanged(int length);

private:
	EditorContext* const _editorContext;
	const StudioModelAssetProvider* const _provider;
	const std::unique_ptr<studiomdl::CStudioModel> _studioModel;
	const std::unique_ptr<graphics::Scene> _scene;

	std::stack<IInputSink*> _inputSinks;
	//TODO: temporary; will need to be set up somewhere else eventually
	std::unique_ptr<camera_operators::CameraOperator> _cameraOperator;
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

	void Save(const QString& fileName, Asset& asset) const override;

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
