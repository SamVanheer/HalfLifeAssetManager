#pragma once

#include "ui_AttachmentsPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class StudioModelAsset;
class StudioModelData;

class AttachmentsPanel final : public DockableWidget
{
public:
	AttachmentsPanel(StudioModelAsset* asset);
	~AttachmentsPanel();

private:
	void UpdateQCString();

private slots:
	void OnAssetChanged(StudioModelAsset* asset);

	void OnSaveSnapshot(StateSnapshot* snapshot);
	void OnLoadSnapshot(StateSnapshot* snapshot);

	void OnAttachmentChanged(int index);
	void OnHighlightAttachmentChanged();

	void OnNameChanged();
	void OnNameRejected();

	void OnTypeChanged();
	void OnBoneChanged();

	void OnOriginChanged();

private:
	Ui_AttachmentsPanel _ui;
	StudioModelAsset* const _asset;
	StudioModelData* _previousModelData{};
};
}
}
