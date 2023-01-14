#pragma once

#include <memory>

#include <QObject>
#include <QString>

#include "application/AssetManager.hpp"
#include "application/SingleInstance.hpp"

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

	static AssetManager* GetApplication() { return _application.get(); }

	int Run(int argc, char* argv[]);

private:
	void ConfigureApplication(const QString& programName);
	
	ParsedCommandLine ParseCommandLine(const QStringList& arguments);

	std::unique_ptr<QSettings> CreateSettings(
		const QString& applicationFileName, const QString& programName, bool isPortable);

	void ConfigureOpenGL(QSettings& settings);

	void CheckOpenGLVersion(const QString& programName, QSettings& settings);

	std::unique_ptr<graphics::IGraphicsContext> InitializeOpenGL();

	bool CheckSingleInstance(const QString& programName, const QString& fileName, QSettings& settings);

	std::unique_ptr<AssetManager> CreateApplication(std::unique_ptr<QSettings> settings,
		std::unique_ptr<graphics::IGraphicsContext> graphicsContext);

private slots:
	void OnExit();

private:
	QApplication* _guiApplication{};

	static inline std::unique_ptr<AssetManager> _application;

	std::unique_ptr<SingleInstance> _singleInstance;
};
