#pragma once

#include <QDialog>
#include <QStandardItemModel>

#include "ui_StudioModelEditEventsDialog.h"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class StudioModelEditEventsDialog final : public QDialog
{
public:
	StudioModelEditEventsDialog(StudioModelAsset* asset, int sequenceIndex, QWidget* parent = nullptr);
	~StudioModelEditEventsDialog();

private slots:
	void accept() override;

	void OnApply();

	void OnDataChanged();

private:
	Ui_StudioModelEditEventsDialog _ui;
	StudioModelAsset* const _asset;
	const int _sequenceIndex;

	QStandardItemModel* _model;
};
}
