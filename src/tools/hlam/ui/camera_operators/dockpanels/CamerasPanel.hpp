#pragma once

#include <string_view>
#include <vector>

#include <QBoxLayout>
#include <QString>
#include <QWidget>

#include "ui_CamerasPanel.h"

namespace ui::camera_operators
{
class CameraOperators;
class SceneCameraOperator;

constexpr std::string_view CameraOperatorPropertyKey{"CameraOperator"};

class CamerasPanel : public QWidget
{
public:
	explicit CamerasPanel(CameraOperators* cameraOperators);
	~CamerasPanel();

	int GetCount() const { return _widgets.size(); }

	QWidget* GetWidget(int index) const;

private:
	void AddCameraOperator(const QString& name, QWidget* widget);

public slots:
	void ChangeCamera(int index);
	void OnLayoutDirectionChanged(QBoxLayout::Direction direction);

private slots:
	void OnChangeCamera(int index);
	void OnAssetCameraChanged(SceneCameraOperator* previous, SceneCameraOperator* current);

	void ChangeToPreviousCamera();
	void ChangeToNextCamera();

private:
	Ui_CamerasPanel _ui;

	CameraOperators* _cameraOperators;

	std::vector<QWidget*> _widgets;
};
}
