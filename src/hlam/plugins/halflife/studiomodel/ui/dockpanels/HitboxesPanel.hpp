#pragma once

#include "ui_HitboxesPanel.h"

#include "ui/DockableWidget.hpp"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

class HitboxesPanel final : public DockableWidget
{
public:
	explicit HitboxesPanel(StudioModelAssetProvider* provider);
	~HitboxesPanel();

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

private:
	void UpdateQCString();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnHitboxChanged(int index);

	void OnHighlightHitboxChanged();

	void OnHitboxPropsChanged();

private:
	Ui_HitboxesPanel _ui;
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};

	bool _changingHitboxProperties{false};
};
}
