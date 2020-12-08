#pragma once

#include <QWidget>

#include "ui_StudioModelAttachmentsPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelAttachmentsPanel final : public QWidget
{
public:
	StudioModelAttachmentsPanel(StudioModelContext* context, QWidget* parent = nullptr);
	~StudioModelAttachmentsPanel();

private:
	void UpdateQCString();

public slots:
	void OnDockPanelChanged(QWidget* current, QWidget* previous);

private slots:
	void OnAttachmentChanged(int index);
	void OnHighlightAttachmentChanged();

	void OnNameChanged();
	void OnTypeChanged();
	void OnBoneChanged();

	void OnOriginChanged();

private:
	Ui_StudioModelAttachmentsPanel _ui;
	StudioModelContext* const _context;

	bool _isActive{false};
};
}
