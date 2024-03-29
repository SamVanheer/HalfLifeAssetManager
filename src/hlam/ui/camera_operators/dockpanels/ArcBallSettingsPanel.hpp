#pragma once

#include <QWidget>

#include "ui_ArcBallSettingsPanel.h"

class ArcBallCameraOperator;

class ArcBallSettingsPanel : public QWidget
{
public:
	ArcBallSettingsPanel(ArcBallCameraOperator* cameraOperator);
	~ArcBallSettingsPanel();

private slots:
	void UpdateCameraProperties();

	void OnFieldOfViewChanged(double value);
	void OnResetFieldOfView();

	void OnPropertyChanged();

	void OnProjectionModeChanged(int index);

private:
	Ui_ArcBallSettingsPanel _ui;
	ArcBallCameraOperator* const _cameraOperator;
};
