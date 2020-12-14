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
#include "ui/EditorContext.hpp"
#include "ui/HLMVMainWindow.hpp"

#include "ui/assets/Assets.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"

#include "ui/options/OptionsPageGameConfigurations.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/options/OptionsPageRegistry.hpp"

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

		defaultFormat.setMajorVersion(2);
		defaultFormat.setMinorVersion(0);
		defaultFormat.setProfile(QSurfaceFormat::OpenGLContextProfile::CompatibilityProfile);

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

	auto optionsPageRegistry{std::make_unique<ui::options::OptionsPageRegistry>()};

	optionsPageRegistry->AddPage(std::make_unique<ui::options::OptionsPageGeneral>());
	optionsPageRegistry->AddPage(std::make_unique<ui::options::OptionsPageGameConfigurations>());

	auto assetProviderRegistry{std::make_unique<ui::assets::AssetProviderRegistry>()};

	assetProviderRegistry->AddProvider(std::make_unique<ui::assets::studiomodel::StudioModelAssetProvider>());

	_editorContext = new ui::EditorContext(std::move(optionsPageRegistry), std::move(assetProviderRegistry), this);

	_mainWindow = new ui::HLMVMainWindow(_editorContext);

	if (!fileName.isEmpty())
	{
		_mainWindow->TryLoadAsset(fileName);
	}

	//Note: must come after the file is loaded or it won't actually show maximized
	_mainWindow->showMaximized();

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
	if (_mainWindow->isMaximized())
	{
		_mainWindow->showMaximized();
	}
	else
	{
		_mainWindow->showNormal();
	}

	_mainWindow->activateWindow();

	_mainWindow->TryLoadAsset(fileName);
}
