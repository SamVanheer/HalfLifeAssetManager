#pragma once

#include <QWidget>

#include "ui_StudioModelBonesPanel.h"

namespace ui
{
class StateSnapshot;
namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelBonesPanel final : public QWidget
{
public:
	StudioModelBonesPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelBonesPanel();

private:
	void InitializeUI();

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
	Ui_StudioModelBonesPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
