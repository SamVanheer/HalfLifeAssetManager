#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

/**
*	@brief Used to create and restore snapshots of an asset's UI state
*/
class StateSnapshot : public QObject
{
public:
	using QObject::QObject;
	~StateSnapshot() = default;

	QStringList AllKeys() const;

	bool Contains(const QString& key) const;

	QVariant Value(const QString& key, const QVariant& defaultValue = {}) const;

	void SetValue(const QString& key, const QVariant& value);

	void Remove(const QString& key);

	void Clear();

private:
	QMap<QString, QVariant> _values;
};

inline QStringList StateSnapshot::AllKeys() const
{
	return _values.uniqueKeys();
}

inline bool StateSnapshot::Contains(const QString& key) const
{
	return _values.contains(key);
}

inline QVariant StateSnapshot::Value(const QString& key, const QVariant& defaultValue) const
{
	return _values.value(key, defaultValue);
}

inline void StateSnapshot::SetValue(const QString& key, const QVariant& value)
{
	_values.insert(key, value);
}

inline void StateSnapshot::Remove(const QString& key)
{
	_values.remove(key);
}

inline void StateSnapshot::Clear()
{
	_values.clear();
}
