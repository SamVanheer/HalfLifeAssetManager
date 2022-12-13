#pragma once

#include "ui_BodyPartsPanel.h"

#include "ui/DockableWidget.hpp"

namespace studiomdl
{
struct BoneController;
}

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class BodyPartsPanel final : public DockableWidget
{
public:
	BodyPartsPanel(StudioModelAsset* asset);
	~BodyPartsPanel();

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

private:
	void UpdateControllerRange(const studiomdl::BoneController& boneController);

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnBodyPartChanged(int index);

	void OnSubmodelChanged(int index);

	void OnSkinChanged(int index);

	void OnModelNameChanged();
	void OnModelNameRejected();

	void OnBoneControllerChanged(int index);

	void OnBoneControllerValueSliderChanged(int value);
	void OnBoneControllerValueSpinnerChanged(double value);

	void OnBoneControllerRangeChanged();

	void OnBoneControllerRestChanged();
	void OnBoneControllerIndexChanged();

	void OnBoneControllerBoneChanged(int index);
	void OnBoneControllerAxisChanged(int index);

private:
	Ui_BodyPartsPanel _ui;
	StudioModelAsset* const _asset;

	float _controllerSliderScale{1.f};
};
}
}
