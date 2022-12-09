#pragma once

#include <QWidget>

#include "ui_StudioModelBodyPartsPanel.h"

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

class StudioModelBodyPartsPanel final : public QWidget
{
public:
	StudioModelBodyPartsPanel(StudioModelAsset* asset);
	~StudioModelBodyPartsPanel();

private:
	void InitializeUI();

	void UpdateControllerRange(const studiomdl::BoneController& boneController);

public slots:
	void OnLayoutDirectionChanged();

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

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
	Ui_StudioModelBodyPartsPanel _ui;
	StudioModelAsset* const _asset;

	float _controllerSliderScale{1.f};
};
}
}
