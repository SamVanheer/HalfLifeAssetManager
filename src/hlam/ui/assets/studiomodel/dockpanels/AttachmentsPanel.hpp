#pragma once

#include "ui_AttachmentsPanel.h"

#include "ui/DockableWidget.hpp"

class StateSnapshot;

namespace studiomodel
{
class StudioModelAsset;
class StudioModelAssetProvider;
class StudioModelData;

class AttachmentsPanel final : public DockableWidget
{
public:
	explicit AttachmentsPanel(StudioModelAssetProvider* provider);
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
	StudioModelAssetProvider* const _provider;
	StudioModelAsset* _asset{};
	StudioModelData* _previousModelData{};

	bool _changingAttachmentProperties{false};
};
}
