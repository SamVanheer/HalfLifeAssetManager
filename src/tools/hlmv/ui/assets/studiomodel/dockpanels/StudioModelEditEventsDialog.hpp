#pragma once

#include <QDialog>
#include <QStandardItemModel>

#include "ui_StudioModelEditEventsDialog.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelEditEventsDialog final : public QDialog
{
public:
	StudioModelEditEventsDialog(StudioModelContext* context, int sequenceIndex, QWidget* parent = nullptr);
	~StudioModelEditEventsDialog();

private slots:
	void accept() override;

	void OnApply();

	void OnDataChanged();

private:
	Ui_StudioModelEditEventsDialog _ui;
	StudioModelContext* const _context;
	const int _sequenceIndex;

	QStandardItemModel* _model;
};
}
