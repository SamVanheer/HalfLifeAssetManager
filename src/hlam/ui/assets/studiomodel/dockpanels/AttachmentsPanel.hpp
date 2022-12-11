#pragma once

#include "ui_AttachmentsPanel.h"

#include "ui/DockableWidget.hpp"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class AttachmentsPanel final : public DockableWidget
{
public:
	AttachmentsPanel(StudioModelAsset* asset);
	~AttachmentsPanel();

private:
	void InitializeUI();

	void UpdateQCString();

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

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
};
}
}
