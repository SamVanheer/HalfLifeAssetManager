#pragma once

#include <QLocalServer>
#include <QObject>
#include <QString>

class SingleInstance final : public QObject
{
	Q_OBJECT

public:
	SingleInstance() = default;
	~SingleInstance() = default;

	SingleInstance(const SingleInstance&) = delete;
	SingleInstance& operator=(const SingleInstance&) = delete;

	/**
	*	@brief Checks if an instance is already running and if so, forwards fileName to it
	*	@return Whether this is the first instance
	*/
	bool Create(const QString& serverName, const QString& fileName = {});

signals:
	void FileNameReceived(const QString& fileName);

private slots:
	void OnNewConnection();

private:
	QLocalServer _server;
};
