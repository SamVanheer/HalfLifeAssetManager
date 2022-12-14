#pragma once

#include "ui_BonesPanel.h"

#include "ui/DockableWidget.hpp"

class QModelIndex;

namespace ui
{
class StateSnapshot;
namespace assets::studiomodel
{
class StudioModelAsset;
class StudioModelData;

class BonesPanel final : public DockableWidget
{
public:
	BonesPanel(StudioModelAsset* asset);
	~BonesPanel();

private:
	void UpdateRootBonesCount();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnBoneChanged(int index);
	void OnHightlightBoneChanged();

	void OnBoneNameChanged();
	void OnBoneNameRejected();

	void OnBoneParentChanged(int index);

	void OnBoneFlagsChanged();

	void OnBonePropertyChanged();

	void OnBoneControllerAxisChanged(int index);

	void OnBoneControllerChanged(int index);

private:
	Ui_BonesPanel _ui;
	StudioModelAsset* const _asset;
	StudioModelData* _previousModelData{};
};
}
}
