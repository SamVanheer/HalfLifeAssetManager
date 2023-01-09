#pragma once

#include <memory>

#include <QObject>
#include <QString>

#include "application/SingleInstance.hpp"

class ApplicationSettings;
class AssetManager;
class QApplication;
class QSettings;
class QStringList;

namespace graphics
{
class IGraphicsContext;
}

struct ParsedCommandLine
{
	bool IsPortable{false};
	bool LogDebugMessagesToConsole{false};
	QString FileName;
};

/**
*	@brief Handles program startup and shutdown
*/
class ToolApplication final : public QObject
{
public:
	ToolApplication();
	~ToolApplication();

	int Run(int argc, char* argv[]);

private:
	void ConfigureApplication(const QString& programName);
	
	ParsedCommandLine ParseCommandLine(const QStringList& arguments);

	std::unique_ptr<QSettings> CreateSettings(const QString& programName, bool isPortable);

	void ConfigureOpenGL(ApplicationSettings& settings);

	bool CheckSingleInstance(const QString& programName, const QString& fileName, ApplicationSettings& settings);

	std::unique_ptr<AssetManager> CreateApplication(std::shared_ptr<ApplicationSettings> applicationSettings,
		std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext);

	std::unique_ptr<graphics::IGraphicsContext> InitializeOpenGL();

private slots:
	void OnExit();

private:
	QApplication* _guiApplication{};

	std::unique_ptr<AssetManager> _application;

	std::unique_ptr<SingleInstance> _singleInstance;
};
