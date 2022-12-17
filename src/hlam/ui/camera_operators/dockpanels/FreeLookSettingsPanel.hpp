#pragma once

#include <QWidget>

#include "ui_FreeLookSettingsPanel.h"

class FreeLookCameraOperator;

class FreeLookSettingsPanel : public QWidget
{
public:
	FreeLookSettingsPanel(FreeLookCameraOperator* cameraOperator);
	~FreeLookSettingsPanel();

private slots:
	void UpdateCameraProperties();

	void OnOriginChanged();

	void OnAnglesChanged();

	void OnFieldOfViewChanged(double value);
	void OnResetFieldOfView();

private:
	Ui_FreeLookSettingsPanel _ui;
	FreeLookCameraOperator* const _cameraOperator;
};
