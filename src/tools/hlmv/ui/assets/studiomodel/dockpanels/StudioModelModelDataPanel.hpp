#pragma once

#include <vector>

#include <QWidget>

#include <glm/vec3.hpp>

#include "ui_StudioModelModelDataPanel.h"

#include "engine/shared/studiomodel/studio.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelModelDataPanel final : public QWidget
{
private:
	struct RootBoneData
	{
		mstudiobone_t* Bone;
		glm::vec3 OriginalRootBonePosition;
	};

public:
	StudioModelModelDataPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelModelDataPanel();

private:
	void UpdateOrigin();

private slots:
	void OnOriginChanged();

	void OnSetOrigin();

	void OnScaleMesh();

	void OnScaleBones();

private:
	Ui_StudioModelModelDataPanel _ui;
	StudioModelAsset* const _asset;

	std::vector<RootBoneData> _rootBonePositions;
};
}
