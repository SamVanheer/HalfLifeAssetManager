#pragma once

#include <QWidget>

#include "ui_FirstPersonSettingsPanel.h"

namespace ui::camera_operators
{
class FirstPersonCameraOperator;

class FirstPersonSettingsPanel : public QWidget
{
public:
	FirstPersonSettingsPanel(FirstPersonCameraOperator* cameraOperator);
	~FirstPersonSettingsPanel();

private slots:
	void OnFieldOfViewChanged(double value);
	void OnResetFieldOfView();

private:
	Ui_FirstPersonSettingsPanel _ui;
	FirstPersonCameraOperator* const _cameraOperator;
};
}
