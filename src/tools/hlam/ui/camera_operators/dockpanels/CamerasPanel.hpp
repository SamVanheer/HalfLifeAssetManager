#pragma once

#include <string_view>
#include <vector>

#include <QString>
#include <QWidget>

#include "ui_CamerasPanel.h"

namespace ui::camera_operators
{
constexpr std::string_view CameraOperatorPropertyKey{"CameraOperator"};

class CamerasPanel : public QWidget
{
	Q_OBJECT

public:
	CamerasPanel(QWidget* parent = nullptr);
	~CamerasPanel();

	int GetCount() const { return _widgets.size(); }

	QWidget* GetWidget(int index) const;

	void AddCameraOperator(const QString& name, QWidget* widget);

signals:
	void CameraChanged(int index);

public slots:
	void ChangeCamera(int index);

private slots:
	void OnChangeCamera(int index);

	void ChangeToPreviousCamera();
	void ChangeToNextCamera();

private:
	Ui_CamerasPanel _ui;

	std::vector<QWidget*> _widgets;
};
}
