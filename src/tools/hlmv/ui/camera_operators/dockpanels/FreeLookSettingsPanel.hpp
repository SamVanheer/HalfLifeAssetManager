#pragma once

#include <QWidget>

#include "ui_FreeLookSettingsPanel.h"

namespace ui::camera_operators
{
class FreeLookCameraOperator;

class FreeLookSettingsPanel : public QWidget
{
public:
	FreeLookSettingsPanel(FreeLookCameraOperator* cameraOperator, QWidget* parent = nullptr);
	~FreeLookSettingsPanel();

private slots:
	void OnFieldOfViewChanged(double value);
	void OnResetFieldOfView();

private:
	Ui_FreeLookSettingsPanel _ui;
	FreeLookCameraOperator* const _cameraOperator;
};
}
