#include <cstdlib>
#include <memory>
#include <stdexcept>

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSettings>
#include <QSurfaceFormat>
#include <QTextStream>

#include "application/AssetList.hpp"
#include "application/AssetManager.hpp"
#include "application/ToolApplication.hpp"

#include "plugins/IAssetManagerPlugin.hpp"
#include "plugins/halflife/HalfLifeAssetManagerPlugin.hpp"
#include "plugins/quake1/Quake1AssetManagerPlugin.hpp"
#include "plugins/source1/Source1AssetManagerPlugin.hpp"

#include "qt/QtLogging.hpp"

#include "settings/ApplicationSettings.hpp"

#include "ui/OpenGLGraphicsContext.hpp"

using namespace logging;

const QString LogBaseFileName{QStringLiteral("HLAM-Log.txt")};

QString LogFileName = LogBaseFileName;

const QtMessageHandler DefaultMessageHandler = qInstallMessageHandler(nullptr);

void FileMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QByteArray localMsg = msg.toLocal8Bit();

	QFile logFile{LogFileName};

	if (!logFile.open(QFile::WriteOnly | QFile::Append))
	{
		QMessageBox::critical(nullptr, "Error", QString{"Couldn't open file \"%1\" for writing log messages"}
			.arg(QFileInfo{logFile}.absoluteFilePath()));
		return;
	}

	QTextStream stream{&logFile};

	const char* messageType = "Unknown";

	switch (type)
	{
	case QtDebugMsg:
		messageType = "Debug";
		break;

	case QtInfoMsg:
		messageType = "Info";
		break;

	case QtWarningMsg:
		messageType = "Warning";
		break;

	case QtCriticalMsg:
		messageType = "Critical";
		break;

	case QtFatalMsg:
		messageType = "Fatal";
		break;
	}

	stream << messageType << ": "
		<< msg << " (" << context.file << ":" << context.line << ", " << context.function << ")\n";

	//Let the default handler handle abort
	/*
	if (type == QtFatalMsg)
	{
		abort();
	}
	*/

	DefaultMessageHandler(type, context, msg);
}

ToolApplication::ToolApplication() = default;
ToolApplication::~ToolApplication() = default;

int ToolApplication::Run(int argc, char* argv[])
{
	try
	{
		const QString programName{QStringLiteral("Half-Life Asset Manager")};

		ConfigureApplication(programName);

		const auto commandLine = ParseCommandLine([&]()
			{
				QStringList args;

				for (int i = 0; i < argc; ++i)
				{
					args << argv[i];
				}

				return args;
			}());

		const auto applicationSettings = std::make_shared<ApplicationSettings>(
			CreateSettings(programName, commandLine.IsPortable).release());

		auto settings = applicationSettings->GetSettings();

		applicationSettings->LoadSettings();

		ConfigureOpenGL(*applicationSettings);

		QApplication app(argc, argv);

		_guiApplication = &app;

		connect(&app, &QApplication::aboutToQuit, this, &ToolApplication::OnExit);

		LogFileName = QApplication::applicationDirPath() + QDir::separator() + LogBaseFileName;

		QFile::remove(LogFileName);

		if (commandLine.LogDebugMessagesToConsole)
		{
			qInstallMessageHandler(&FileMessageOutput);
		}

		if (CheckSingleInstance(programName, commandLine.FileName, *applicationSettings))
		{
			return EXIT_SUCCESS;
		}

		{
			const auto openGLFormat = QOpenGLContext::globalShareContext()->format();

			auto makeVersionCode = [](int major, int minor)
			{
				return (major << 8) + minor;
			};

			const int versionCode = makeVersionCode(openGLFormat.majorVersion(), openGLFormat.minorVersion());

			const int minimumSupportedVersionCode = makeVersionCode(2, 1);

			//Only check this once
			if (!settings->value("Video/CheckedOpenGLVersion", false).toBool()
				&& versionCode < minimumSupportedVersionCode)
			{
				QMessageBox::warning(nullptr, "Warning",
					QString{"%1 may not work correctly with your version of OpenGL (%2.%3)"}
						.arg(programName)
						.arg(openGLFormat.majorVersion())
						.arg(openGLFormat.minorVersion()));

				settings->setValue("Video/CheckedOpenGLVersion", true);
			}
		}

		auto offscreenContext{InitializeOpenGL()};

		if (!offscreenContext)
		{
			return EXIT_FAILURE;
		}

		_application = CreateApplication(applicationSettings, std::move(offscreenContext));

		if (!_application)
		{
			return EXIT_FAILURE;
		}

		if (_singleInstance)
		{
			connect(_singleInstance.get(), &SingleInstance::FileNameReceived,
				_application.get(), &AssetManager::OnFileNameReceived);
		}

		_application->Start();

		if (!commandLine.FileName.isEmpty())
		{
			_application->GetAssets()->TryLoad(commandLine.FileName);
		}

		return app.exec();
	}
	catch (const std::exception& e)
	{
		qDebug() << "Unhandled exception:" << e.what();
		throw;
	}
}

void ToolApplication::ConfigureApplication(const QString& programName)
{
	QApplication::setOrganizationName(programName);
	QApplication::setOrganizationDomain(QStringLiteral("https://github.com/Solokiller/HL_Tools"));
	QApplication::setApplicationName(programName);
	QApplication::setApplicationDisplayName(programName);

	QSettings::setDefaultFormat(QSettings::Format::IniFormat);
}

ParsedCommandLine ToolApplication::ParseCommandLine(const QStringList& arguments)
{
	QCommandLineParser parser;

	parser.addOption(QCommandLineOption{"portable", "Launch in portable mode"});
	parser.addOption(QCommandLineOption{"log-to-file", "Log debug messages to a file"});
	parser.addPositionalArgument("fileName", "Filename of the model to load on startup", "[fileName]");

	parser.process(arguments);

	ParsedCommandLine result;

	result.IsPortable = parser.isSet("portable");
	result.LogDebugMessagesToConsole = parser.isSet("log-to-file");

	const auto positionalArguments = parser.positionalArguments();

	if (!positionalArguments.empty())
	{
		result.FileName = positionalArguments[0];

		// Check if this filename is valid.
		// If not, try to combine all positional arguments into a single filename.
		// If that's valid, use it. Otherwise use only the first argument and let error handling deal with it.
		if (!QFile::exists(result.FileName))
		{
			result.FileName = positionalArguments.join(" ");

			if (!QFile::exists(result.FileName))
			{
				result.FileName = positionalArguments[0];
			}
		}
	}

	return result;
}

std::unique_ptr<QSettings> ToolApplication::CreateSettings(const QString& programName, bool isPortable)
{
	if (isPortable)
	{
		const QString directory = QApplication::applicationDirPath();
		const QString fileName = QString{"%1/%2.ini"}.arg(directory).arg(programName);
		return std::make_unique<QSettings>(fileName, QSettings::Format::IniFormat);
	}
	else
	{
		return std::make_unique<QSettings>();
	}
}

void ToolApplication::ConfigureOpenGL(ApplicationSettings& settings)
{
	//Neither OpenGL ES nor Software OpenGL will work here
	QApplication::setAttribute(Qt::ApplicationAttribute::AA_UseDesktopOpenGL, true);
	QApplication::setAttribute(Qt::ApplicationAttribute::AA_ShareOpenGLContexts, true);

	//Set up the OpenGL surface settings to match the Half-Life engine's requirements
	//Vanilla Half-Life uses OpenGL 1.0 for game rendering
	//TODO: eventually an option should be added to allow switching to 3.3 for shader based rendering
	QSurfaceFormat::FormatOptions formatOptions{QSurfaceFormat::FormatOption::DeprecatedFunctions};

#ifdef _DEBUG
	formatOptions.setFlag(QSurfaceFormat::FormatOption::DebugContext, true);
#endif

	QSurfaceFormat defaultFormat{formatOptions};

	defaultFormat.setMajorVersion(1);
	defaultFormat.setMinorVersion(0);
	defaultFormat.setProfile(QSurfaceFormat::OpenGLContextProfile::CompatibilityProfile);

	defaultFormat.setDepthBufferSize(24);
	defaultFormat.setStencilBufferSize(8);
	defaultFormat.setSwapBehavior(QSurfaceFormat::SwapBehavior::DoubleBuffer);
	defaultFormat.setRedBufferSize(4);
	defaultFormat.setGreenBufferSize(4);
	defaultFormat.setBlueBufferSize(4);
	defaultFormat.setAlphaBufferSize(4);
	defaultFormat.setSwapInterval(settings.ShouldEnableVSync() ? 1 : 0);

	qCDebug(logging::HLAM) << "Configuring OpenGL for" << defaultFormat;

	QSurfaceFormat::setDefaultFormat(defaultFormat);
}

bool ToolApplication::CheckSingleInstance(
	const QString& programName, const QString& fileName, ApplicationSettings& settings)
{
	if (settings.ShouldUseSingleInstance())
	{
		_singleInstance.reset(new SingleInstance());

		if (!_singleInstance->Create(programName, fileName))
		{
			return true;
		}
	}

	return false;
}

std::unique_ptr<AssetManager> ToolApplication::CreateApplication(
	std::shared_ptr<ApplicationSettings> applicationSettings,
	std::unique_ptr<graphics::IGraphicsContext>&& graphicsContext)
{
	auto application = std::make_unique<AssetManager>(_guiApplication, applicationSettings, std::move(graphicsContext));

	{
		bool success = true;

		success = application->AddPlugin(std::make_unique<HalfLifeAssetManagerPlugin>()) && success;
		success = application->AddPlugin(std::make_unique<Quake1AssetManagerPlugin>()) && success;
		success = application->AddPlugin(std::make_unique<Source1AssetManagerPlugin>()) && success;

		if (!success)
		{
			return {};
		}
	}

	return application;
}

std::unique_ptr<graphics::IGraphicsContext> ToolApplication::InitializeOpenGL()
{
	auto context{std::make_unique<QOpenGLContext>()};

	context->setFormat(QSurfaceFormat::defaultFormat());

	const auto shareContext{QOpenGLContext::globalShareContext()};

	context->setShareContext(shareContext);
	context->setScreen(shareContext->screen());

	if (!context->create())
	{
		QMessageBox::critical(nullptr, "Fatal Error", "Couldn't create OpenGL context");
		return {};
	}

	auto surface{std::make_unique<QOffscreenSurface>(context->screen(), this)};

	surface->setFormat(context->format());
	surface->setScreen(context->screen());
	surface->create();

	if (!context->makeCurrent(surface.get()))
	{
		QMessageBox::critical(nullptr, "Fatal Error", "Couldn't make offscreen surface context current");
		return {};
	}

	return std::make_unique<OpenGLGraphicsContext>(std::move(surface), std::move(context));
}

void ToolApplication::OnExit()
{
	_application->OnExit();

	_application.reset();
	_singleInstance.reset();
}
