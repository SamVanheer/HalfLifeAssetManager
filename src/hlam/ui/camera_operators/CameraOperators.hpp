#pragma once

#include <cassert>

#include <QList>
#include <QObject>

#include "CameraOperator.hpp"

/**
*	@brief Manages the set of camera operators, signals when the operator changes
*/
class CameraOperators : public QObject
{
	Q_OBJECT

public:
	using QObject::QObject;
	~CameraOperators() = default;

	QList<SceneCameraOperator*> GetAll() const { return _cameraOperators; }

	int Count() const { return _cameraOperators.size(); }

	SceneCameraOperator* Get(int index) const
	{
		return _cameraOperators[index];
	}

	bool Contains(SceneCameraOperator* cameraOperator) const
	{
		return _cameraOperators.contains(cameraOperator);
	}

	int IndexOf(SceneCameraOperator* cameraOperator) const
	{
		return _cameraOperators.indexOf(cameraOperator);
	}

	void Add(SceneCameraOperator* cameraOperator)
	{
		assert(cameraOperator);

		if (!cameraOperator)
		{
			return;
		}

		_cameraOperators.append(cameraOperator);

		cameraOperator->setParent(this);
	}

	SceneCameraOperator* GetCurrent() const { return _current; }

	void SetCurrent(SceneCameraOperator* cameraOperator);

	void PreviousCamera();
	void NextCamera();

private:
	void ChangeCamera(bool next);

signals:
	void CameraChanged(SceneCameraOperator* previous, SceneCameraOperator* current);

private:
	QList<SceneCameraOperator*> _cameraOperators;

	SceneCameraOperator* _current = nullptr;
};

inline void CameraOperators::SetCurrent(SceneCameraOperator* cameraOperator)
{
	if (cameraOperator)
	{
		if (!Contains(cameraOperator))
		{
			assert(!"Tried to set a camera operator not owned by the camera operators manager");
			return;
		}
	}

	if (_current != cameraOperator)
	{
		auto previous = _current;
		_current = cameraOperator;

		emit CameraChanged(previous, _current);
	}
}

inline void CameraOperators::PreviousCamera()
{
	ChangeCamera(false);
}

inline void CameraOperators::NextCamera()
{
	ChangeCamera(true);
}

inline void CameraOperators::ChangeCamera(bool next)
{
	int index;

	if (_cameraOperators.size() == 0)
	{
		index = -1;
	}
	else if (!_current)
	{
		index = next ? 0 : _cameraOperators.size() - 1;
	}
	else
	{
		auto it = std::find_if(_cameraOperators.begin(), _cameraOperators.end(), [this](const auto& candidate)
			{
				return candidate == _current;
			});

		index = it - _cameraOperators.begin();

		if (next)
		{
			index += 1;
		}
		else
		{
			index -= 1;
		}

		if (index < 0)
		{
			index = _cameraOperators.size() - 1;
		}
		else if (index >= _cameraOperators.size())
		{
			index = 0;
		}
	}

	SetCurrent(index != -1 ? _cameraOperators[index] : nullptr);
}
