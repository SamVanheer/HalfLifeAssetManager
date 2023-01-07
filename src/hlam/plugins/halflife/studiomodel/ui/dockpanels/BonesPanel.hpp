#pragma once

#include "ui_BonesPanel.h"

#include "ui/DockableWidget.hpp"

class QModelIndex;

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

class BonesPanel final : public DockableWidget
{
public:
	explicit BonesPanel(StudioModelAssetProvider* provider);
	~BonesPanel();

private:
	void UpdateRootBonesCount();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnBoneChanged(int index);
	void OnHightlightBoneChanged();

	void OnBoneNameRejected();

	void OnPropsChanged();

private:
	Ui_BonesPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};

	bool _changingBoneProperties{false};
};
}
