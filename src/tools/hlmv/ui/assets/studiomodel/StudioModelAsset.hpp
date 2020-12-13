#pragma once

#include <cassert>
#include <memory>
#include <stack>

#include <QColor>
#include <QObject>

#include "engine/shared/studiomodel/CStudioModel.h"

#include "graphics/Scene.hpp"

#include "ui/assets/Assets.hpp"

namespace ui
{
class IInputSink;

namespace assets::studiomodel
{
class StudioModelAssetProvider;

class StudioModelAsset final : public Asset
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

	QWidget* CreateEditWidget(EditorContext* editorContext) override;

	void SetupFullscreenWidget(EditorContext* editorContext, FullscreenWidget* fullscreenWidget) override;

	void Save(const QString& fileName) override;

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
	void SetBackgroundColor(QColor color)
	{
		_scene->SetBackgroundColor({color.redF(), color.greenF(), color.blueF()});
	}

private:
	EditorContext* const _editorContext;
	const StudioModelAssetProvider* const _provider;
	const std::unique_ptr<studiomdl::CStudioModel> _studioModel;
	const std::unique_ptr<graphics::Scene> _scene;

	std::stack<IInputSink*> _inputSinks;
};

class StudioModelAssetProvider final : public IAssetProvider
{
public:
	entt::id_type GetAssetType() const override { return entt::type_index<StudioModelAsset>::value(); }

	bool CanLoad(const QString& fileName) const override;

	std::unique_ptr<Asset> Load(EditorContext* editorContext, const QString& fileName) const override;

	void Save(const QString& fileName, Asset& asset) const override;

	void Save(const QString& fileName, StudioModelAsset& asset) const;
};

inline const IAssetProvider* StudioModelAsset::GetProvider() const
{
	return _provider;
}
}
}
