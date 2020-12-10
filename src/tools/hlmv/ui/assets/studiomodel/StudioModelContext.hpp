#pragma once

#include <cassert>
#include <memory>
#include <stack>

#include <QColor>
#include <QObject>

#include "graphics/Scene.hpp"

namespace ui
{
class EditorContext;
class IInputSink;

namespace assets::studiomodel
{
class StudioModelAsset;

/**
*	@brief Stores shared UI state for studio models
*/
class StudioModelContext final : public QObject
{
	Q_OBJECT

public:
	StudioModelContext(EditorContext* editorContext, StudioModelAsset* asset, std::unique_ptr<graphics::Scene>&& scene, QObject* parent = nullptr)
		: QObject(parent)
		, _editorContext(editorContext)
		, _asset(asset)
		, _scene(std::move(scene))
	{
	}

	~StudioModelContext() = default;

	EditorContext* GetEditorContext() { return _editorContext; }

	StudioModelAsset* GetAsset() { return _asset; }

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
	StudioModelAsset* const _asset;
	const std::unique_ptr<graphics::Scene> _scene;

	std::stack<IInputSink*> _inputSinks;
};
}
}
