#pragma once

#include <QWidget>

#include "ui_StudioModelAttachmentsPanel.h"

namespace ui::assets::studiomodel
{
class ModelChangeEvent;
class StudioModelAsset;

class StudioModelAttachmentsPanel final : public QWidget
{
public:
	StudioModelAttachmentsPanel(StudioModelAsset* asset, QWidget* parent = nullptr);
	~StudioModelAttachmentsPanel();

private:
	void UpdateQCString();

public slots:
	void OnDockPanelChanged(QWidget* current, QWidget* previous);

private slots:
	void OnModelChanged(const ModelChangeEvent& event);

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

	bool _isActive{false};
};
}
