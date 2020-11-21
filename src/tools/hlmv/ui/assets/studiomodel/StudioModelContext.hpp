#pragma once

#include <QColor>
#include <QObject>

#include "graphics/Scene.hpp"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

/**
*	@brief Stores shared UI state for studio models
*/
class StudioModelContext final : public QObject
{
	Q_OBJECT

public:
	StudioModelContext(StudioModelAsset* asset, graphics::Scene* scene, QObject* parent = nullptr)
		: QObject(parent)
		, _asset(asset)
		, _scene(scene)
	{
	}

	~StudioModelContext() = default;

	StudioModelAsset* GetAsset() { return _asset; }

	graphics::Scene* GetScene() { return _scene; }

signals:
	void Tick();

public slots:
	void SetBackgroundColor(QColor color)
	{
		_scene->SetBackgroundColor({color.redF(), color.greenF(), color.blueF()});
	}

private:
	StudioModelAsset* const _asset;
	graphics::Scene* const _scene;
};
}
