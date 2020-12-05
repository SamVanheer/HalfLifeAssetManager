#pragma once

#include <QString>
#include <QUuid>
#include <QVariant>

namespace ui::options
{
/**
*	@brief Defines a mod
*/
class GameConfiguration final
{
public:
	GameConfiguration(const QUuid& id, QString&& directory, QString&& name)
		:  _id(id)
		, _directory(std::move(directory))
		, _name(std::move(name))
	{
	}

	GameConfiguration(const GameConfiguration&) = default;
	GameConfiguration& operator=(const GameConfiguration&) = delete;

	~GameConfiguration() = default;

	const QUuid& GetId() const { return _id; }

	QString GetDirectory() const { return _directory; }

	void SetDirectory(QString&& value)
	{
		_directory = std::move(value);
	}

	QString GetName() const { return _name; }

	void SetName(QString&& value)
	{
		_name = std::move(value);
	}

private:
	const QUuid _id;
	QString _directory;
	QString _name;
};
}

Q_DECLARE_METATYPE(ui::options::GameConfiguration*)
