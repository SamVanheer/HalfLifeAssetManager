#pragma once

#include <QWidget>

#include "ui_StudioModelBodyPartsPanel.h"

namespace ui::assets::studiomodel
{
class StudioModelContext;

class StudioModelBodyPartsPanel final : public QWidget
{
public:
	StudioModelBodyPartsPanel(StudioModelContext* context, QWidget* parent = nullptr);
	~StudioModelBodyPartsPanel();

private:
	void UpdateControllerRange();

private slots:
	void OnBodyPartChanged(int index);

	void OnSubmodelChanged(int index);

	void OnSkinChanged(int index);

	void OnBoneControllerChanged(int index);

	void OnBoneControllerValueSliderChanged(int value);
	void OnBoneControllerValueSpinnerChanged(double value);

	void OnBoneControllerBoneChanged(int index);
	void OnBoneControllerRangeChanged();

	void OnBoneControllerRestChanged();
	void OnBoneControllerIndexChanged();

	void OnBoneControllerTypeChanged(int index);

private:
	Ui_StudioModelBodyPartsPanel _ui;
	StudioModelContext* const _context;

	float _controllerSliderScale{1.f};
};
}
