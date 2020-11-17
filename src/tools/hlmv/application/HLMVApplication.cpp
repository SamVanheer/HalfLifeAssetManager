#include <cstdlib>
#include <memory>

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QScopedPointer>
#include <QSettings>
#include <QStyleFactory>
#include <QSurfaceFormat>

#include "application/HLMVApplication.hpp"
#include "ui/HLMVMainWindow.hpp"

#include "ui/assets/Assets.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"

int HLMVApplication::Run(int argc, char* argv[])
{
	const QString programName{"Half-Life Model Viewer"};

	QApplication::setOrganizationName(programName);
	QApplication::setOrganizationDomain("https://github.com/Solokiller/HL_Tools");
	QApplication::setApplicationName(programName);
	QApplication::setApplicationDisplayName(programName);

	QSettings::setDefaultFormat(QSettings::Format::IniFormat);

	QApplication::setAttribute(Qt::ApplicationAttribute::AA_ShareOpenGLContexts, true);

	//Set up the OpenGL surface settings to match the Half-Life engine's requirements
	//Vanilla Half-Life uses OpenGL 1.0 for game rendering
	//TODO: eventually an option should be added to allow switching to 3.3 for shader based rendering
	{
		QSurfaceFormat defaultFormat(QSurfaceFormat::FormatOption::DebugContext | QSurfaceFormat::FormatOption::DeprecatedFunctions);

		defaultFormat.setMajorVersion(3);
		defaultFormat.setMinorVersion(3);
		defaultFormat.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);

		defaultFormat.setDepthBufferSize(24);
		defaultFormat.setStencilBufferSize(8);
		defaultFormat.setSwapBehavior(QSurfaceFormat::SwapBehavior::DoubleBuffer);
		defaultFormat.setRedBufferSize(4);
		defaultFormat.setGreenBufferSize(4);
		defaultFormat.setBlueBufferSize(4);
		defaultFormat.setAlphaBufferSize(0);

		QSurfaceFormat::setDefaultFormat(defaultFormat);
	}

	QApplication app(argc, argv);

	connect(&app, &QApplication::aboutToQuit, this, &HLMVApplication::OnExit);

	QSettings settings;

	//TODO: load settings
	//TODO: improve settings loading to avoid having it be this messy
	settings.beginGroup("startup");
	const auto useSingleInstance = settings.value("useSingleInstance", false).toBool();
	settings.endGroup();

	QString fileName;

	{
		QCommandLineParser parser;

		parser.addPositionalArgument("fileName", "Filename of the model to load on startup", "[fileName]");

		parser.process(app);

		const auto positionalArguments = parser.positionalArguments();

		if (!positionalArguments.empty())
		{
			fileName = positionalArguments[0];
		}
	}

	if (useSingleInstance)
	{
		singleInstance.reset(new SingleInstance());

		if (!singleInstance->Create(programName, fileName))
		{
			return EXIT_SUCCESS;
		}

		connect(singleInstance.get(), &SingleInstance::FileNameReceived, this, &HLMVApplication::OnFileNameReceived);
	}

	auto assetProviderRegistry = std::make_unique<ui::assets::AssetProviderRegistry>();

	assetProviderRegistry->AddProvider(std::make_unique<ui::assets::studiomodel::StudioModelAssetProvider>());

	_mainWindow = new ui::HLMVMainWindow(std::move(assetProviderRegistry));

	//TODO: doesn't actually show maximized
	_mainWindow->showMaximized();

	//TODO: open file specified on command line

	return app.exec();
}

void HLMVApplication::OnExit()
{
	_mainWindow = nullptr;

	if (singleInstance)
	{
		singleInstance.reset();
	}
}

void HLMVApplication::OnFileNameReceived(const QString& fileName)
{
	//TODO: load file

	if (_mainWindow->isMaximized())
	{
		_mainWindow->showMaximized();
	}
	else
	{
		_mainWindow->showNormal();
	}

	_mainWindow->activateWindow();
}
