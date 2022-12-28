#pragma once

#include "ui_BodyPartsPanel.h"

#include "ui/DockableWidget.hpp"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

class BodyPartsPanel final : public DockableWidget
{
public:
	explicit BodyPartsPanel(StudioModelAssetProvider* provider);
	~BodyPartsPanel();

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnBodyPartChanged(int index);

	void OnSubmodelChanged(int index);

	void OnSkinChanged(int index);

	void OnModelNameChanged();
	void OnModelNameRejected();

private:
	Ui_BodyPartsPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};

	bool _changingBoneControllerProperties{false};
};
}
