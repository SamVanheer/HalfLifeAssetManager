#pragma once

#include "ui_BoneControllersPanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomdl
{
struct BoneController;
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

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

private:
	void UpdateControllerRange(const studiomdl::BoneController& boneController);

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnBoneControllerChanged(int index);

	void OnBoneControllerValueSliderChanged(int value);
	void OnBoneControllerValueSpinnerChanged(double value);

	void OnBoneControllerRangeChanged();

	void OnBoneControllerRestChanged();
	void OnBoneControllerIndexChanged();

	void OnBoneControllerBoneChanged(int index);
	void OnBoneControllerAxisChanged(int index);

private:
	Ui_BoneControllersPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};

	bool _changingBoneControllerProperties{false};

	float _controllerSliderScale{1.f};
};
}
