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

private slots:
	void OnBodyPartChanged(int index);

	void OnSubmodelChanged(int index);

	void OnSkinChanged(int index);

	void OnBoneControllerChanged(int index);

	void OnBoneControllerValueChanged(int value);

private:
	Ui_StudioModelBodyPartsPanel _ui;
	StudioModelContext* const _context;

	float _controllerSliderScale{1.f};
};
}
