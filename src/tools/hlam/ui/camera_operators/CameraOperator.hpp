#pragma once

#include <cassert>

#include <QMouseEvent>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QWheelEvent>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "graphics/Camera.hpp"

#include "ui/settings/GeneralSettings.hpp"

class QWidget;

namespace ui::camera_operators
{
class CameraOperator : public QObject
{
	Q_OBJECT

public:
	CameraOperator(settings::GeneralSettings* generalSettings)
		: _generalSettings(generalSettings)
	{
		assert(_generalSettings);
	}

	virtual ~CameraOperator() {}

	graphics::Camera* GetCamera() { return &_camera; }

	virtual QString GetName() const = 0;

	virtual QWidget* CreateEditWidget() = 0;

	virtual void MouseEvent(QMouseEvent& event) = 0;

	virtual void WheelEvent(QWheelEvent& event)
	{
		event.ignore();
	}

	virtual void CenterView(const glm::vec3& targetOrigin, const glm::vec3& cameraOrigin, float pitch, float yaw) = 0;

	virtual void SaveView() = 0;

	virtual void RestoreView() = 0;

protected:
	float GetMouseXValue(float value)
	{
		if (_generalSettings->ShouldInvertMouseX())
		{
			value = -value;
		}

		value *= _generalSettings->GetNormalizedMouseSensitivity();

		return value;
	}

	float GetMouseYValue(float value)
	{
		if (_generalSettings->ShouldInvertMouseY())
		{
			value = -value;
		}

		value *= _generalSettings->GetNormalizedMouseSensitivity();

		return value;
	}

signals:
	void CameraPropertiesChanged();

protected:
	const settings::GeneralSettings* const _generalSettings;

	glm::vec2 _oldCoordinates{0.f};
	Qt::MouseButtons _trackedButtons{};
	graphics::Camera _camera;
};
}

Q_DECLARE_METATYPE(ui::camera_operators::CameraOperator*)
