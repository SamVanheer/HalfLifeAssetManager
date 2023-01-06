#pragma once

#include <string_view>
#include <vector>

#include <QBoxLayout>
#include <QString>
#include <QWidget>

#include "ui_CamerasPanel.h"

#include "ui/DockableWidget.hpp"

class CameraOperators;
class SceneCameraOperator;

constexpr std::string_view CameraOperatorPropertyKey{"CameraOperator"};

class CamerasPanel : public DockableWidget
{
public:
	explicit CamerasPanel(CameraOperators* cameraOperators);
	~CamerasPanel();

	void OnLayoutDirectionChanged(QBoxLayout::Direction direction) override;

	int GetCount() const { return _widgets.size(); }

	QWidget* GetWidget(int index) const;

private:
	void AddCameraOperator(const QString& name, QWidget* widget);

public slots:
	void ChangeCamera(int index);

private slots:
	void OnChangeCamera(int index);
	void OnAssetCameraChanged(SceneCameraOperator* previous, SceneCameraOperator* current);

	void ChangeToPreviousCamera();
	void ChangeToNextCamera();

private:
	Ui_CamerasPanel _ui;

	CameraOperators* const _cameraOperators;

	std::vector<QWidget*> _widgets;
};
