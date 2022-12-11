#pragma once

#include <QWidget>

#include "ui_ArcBallSettingsPanel.h"

namespace ui::camera_operators
{
class ArcBallCameraOperator;

class ArcBallSettingsPanel : public QWidget
{
public:
	ArcBallSettingsPanel(ArcBallCameraOperator* cameraOperator);
	~ArcBallSettingsPanel();

private slots:
	void UpdateCameraProperties();

	void OnPropertyChanged();

	void OnFieldOfViewChanged(double value);
	void OnResetFieldOfView();

private:
	Ui_ArcBallSettingsPanel _ui;
	ArcBallCameraOperator* const _cameraOperator;
};
}
