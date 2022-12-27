#pragma once

#include <QObject>
#include <QString>
#include <QUuid>
#include <QVariant>

/**
*	@brief Defines a mod
*/
class GameConfiguration final : public QObject
{
	Q_OBJECT

public:
	GameConfiguration(const QUuid& id, QString&& directory, QString&& name, QObject* parent = nullptr)
		:  QObject(parent)
		, _id(id)
		, _directory(std::move(directory))
		, _name(std::move(name))
	{
	}

	GameConfiguration(const GameConfiguration& other)
		: QObject()
		, _id(other._id)
		, _directory(other._directory)
		, _name(other._name)
	{
		this->setParent(other.parent());
	}

	GameConfiguration& operator=(const GameConfiguration&) = delete;

	~GameConfiguration() = default;

	const QUuid& GetId() const { return _id; }

	QString GetDirectory() const { return _directory; }

	void SetDirectory(QString&& value)
	{
		if (_directory != value)
		{
			_directory = std::move(value);

			emit DirectoryChanged(_directory);
		}
	}

	QString GetName() const { return _name; }

	void SetName(QString&& value)
	{
		_name = std::move(value);
	}

signals:
	void DirectoryChanged(const QString& directory);

private:
	const QUuid _id;
	QString _directory;
	QString _name;
};

Q_DECLARE_METATYPE(GameConfiguration*)
