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

#include "settings/ApplicationSettings.hpp"

#include "ui/StateSnapshot.hpp"

class QWidget;

class CameraOperator : public QObject, public graphics::ICameraOperator
{
public:
	virtual ~CameraOperator() = default;

	graphics::Camera* GetCamera() override final { return &_camera; }

	virtual void MouseEvent(QMouseEvent& event) = 0;

	virtual void WheelEvent(QWheelEvent& event)
	{
		event.ignore();
	}

protected:
	graphics::Camera _camera;
};

class SceneCameraOperator : public CameraOperator
{
	Q_OBJECT

public:
	SceneCameraOperator(ApplicationSettings* applicationSettings)
		: _applicationSettings(applicationSettings)
	{
		assert(_applicationSettings);
	}

	virtual QString GetName() const = 0;

	virtual QWidget* CreateEditWidget() = 0;

	virtual void CenterView(const glm::vec3& targetOrigin, const glm::vec3& cameraOrigin, float pitch, float yaw) = 0;

	virtual bool SaveView(StateSnapshot* snapshot)
	{
		return false;
	}

	virtual void RestoreView(StateSnapshot* snapshot) {}

protected:
	float GetMouseXValue(float value)
	{
		if (_applicationSettings->ShouldInvertMouseX())
		{
			value = -value;
		}

		value *= _applicationSettings->GetNormalizedMouseSensitivity();

		return value;
	}

	float GetMouseYValue(float value)
	{
		if (_applicationSettings->ShouldInvertMouseY())
		{
			value = -value;
		}

		value *= _applicationSettings->GetNormalizedMouseSensitivity();

		return value;
	}

signals:
	void CameraPropertiesChanged();

protected:
	const ApplicationSettings* const _applicationSettings;

	glm::vec2 _oldCoordinates{0.f};
	Qt::MouseButtons _trackedButtons{};
};

Q_DECLARE_METATYPE(SceneCameraOperator*)
