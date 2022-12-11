#pragma once

#include "ui_BonesPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;
namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class BonesPanel final : public DockableWidget
{
public:
	BonesPanel(StudioModelAsset* asset);
	~BonesPanel();

private:
	void InitializeUI();

	void UpdateRootBonesCount();

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

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
};
}
}
