#pragma once

#include "ui_BoneControllersPanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomdl
{
struct StudioBoneController;
}

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

class BoneControllersPanel final : public DockableWidget
{
public:
	explicit BoneControllersPanel(StudioModelAssetProvider* provider);
	~BoneControllersPanel();

private:
	void UpdateControllerRange(const studiomdl::StudioBoneController& boneController);

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnBoneControllerChanged(int index);

	void OnBoneControllerValueSliderChanged(int value);
	void OnBoneControllerValueSpinnerChanged(double value);

	void OnPropsChanged();

private:
	Ui_BoneControllersPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};

	bool _changingBoneControllerProperties{false};

	float _controllerSliderScale{1.f};
};
}
