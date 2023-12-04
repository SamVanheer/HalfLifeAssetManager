#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include <QObject>

/**
*	@brief Base class for ObservableList. Provides signals to connect to.
*/
class ObservableListBase : public QObject
{
	Q_OBJECT

protected:
	ObservableListBase() = default;

signals:
	void ObjectAdded(int index);
	void AboutToRemoveObject(int index);
	void ObjectRemoved(int index);
};

/**
*	@brief Dynamic array that emits signals on object addition and removal.
*/
template<typename T>
class ObservableList final : public ObservableListBase
{
public:
	ObservableList() = default;

	std::size_t Count() const { return _objects.size(); }

	int IndexOf(const T& obj) const;

	template<typename Predicate>
	int IndexOf(Predicate predicate) const;

	const T& Get(std::size_t index) const { return _objects[index]; }

	T& Get(std::size_t index) { return _objects[index]; }

	void Add(T&& obj);

	void Remove(int index);

private:
	std::vector<T> _objects;
};

template<typename T>
int ObservableList<T>::IndexOf(const T& obj) const
{
	if (auto it = std::find_if(_objects.begin(), _objects.end(),
		[&](const auto& candidate)
		{
			return candidate == obj;
		});
		it != _objects.end())
	{
		return it - _objects.begin();
	}

	return -1;
}

template<typename T>
template<typename Predicate>
int ObservableList<T>::IndexOf(Predicate predicate) const
{
	if (auto it = std::find_if(_objects.begin(), _objects.end(), predicate); it != _objects.end())
	{
		return it - _objects.begin();
	}

	return -1;
}

template<typename T>
void ObservableList<T>::Add(T&& obj)
{
	_objects.push_back(std::move(obj));

	emit ObjectAdded(_objects.size() - 1);
}

template<typename T>
void ObservableList<T>::Remove(int index)
{
	if (index < 0 || static_cast<size_t>(index) >= _objects.size())
	{
		assert(!"Invalid object index");
		return;
	}

	emit AboutToRemoveObject(index);

	// Don't destroy the object until after we've cleaned everything up.
	const T obj = std::move(_objects[index]);

	_objects.erase(_objects.begin() + index);

	emit ObjectRemoved(index);
}
