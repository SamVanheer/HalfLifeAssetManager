#pragma once

#include <QWidget>

#include "ui_StudioModelAttachmentsPanel.h"

namespace ui
{
class StateSnapshot;

namespace assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelAttachmentsPanel final : public QWidget
{
public:
	StudioModelAttachmentsPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelAttachmentsPanel();

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
	Ui_StudioModelAttachmentsPanel _ui;
	StudioModelAsset* const _asset;
};
}
}
