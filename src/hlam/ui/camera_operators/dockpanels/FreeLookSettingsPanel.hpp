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

	void OnFieldOfViewChanged(double value);
	void OnResetFieldOfView();

	void OnAnglesChanged();

	void OnOriginChanged();

private:
	Ui_FreeLookSettingsPanel _ui;
	FreeLookCameraOperator* const _cameraOperator;
};
