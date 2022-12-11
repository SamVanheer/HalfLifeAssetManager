#include <QLocalSocket>

#include "application/SingleInstance.hpp"

bool SingleInstance::Create(const QString& serverName, const QString& fileName)
{
	//Check if an instance is already running
	QLocalSocket socket;

	socket.connectToServer(serverName, QIODevice::OpenModeFlag::WriteOnly);

	if (socket.waitForConnected(1000))
	{
		//If a filename was specified, this will open the file in the existing instance
		//Otherwise, the existing instance can choose to set focus on itself
		socket.write(fileName.toStdString().c_str());

		socket.waitForBytesWritten(-1);
		socket.waitForDisconnected(-1);

		return false;
	}

	connect(&_server, &QLocalServer::newConnection, this, &SingleInstance::OnNewConnection);

	_server.listen(serverName);

	return true;
}

void SingleInstance::OnNewConnection()
{
	auto socket = _server.nextPendingConnection();

	if (socket->waitForReadyRead(1000))
	{
		const auto buffer = socket->readAll();

		const QString fileName{buffer};

		emit FileNameReceived(fileName);
	}

	socket->close();
}
