#include <cassert>

#include <QFileDialog>

#include "settings/ApplicationSettings.hpp"

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"
#include "ui/options/OptionsPageGeneral.hpp"

const QString OptionsPageExternalProgramsId{QStringLiteral("E.ExternalPrograms")};
const QString ExternalProgramsExeFilter{QStringLiteral("Executable Files (*.exe *.com);;All Files (*.*)")};

OptionsPageExternalPrograms::OptionsPageExternalPrograms(const std::shared_ptr<ApplicationSettings>& applicationSettings)
	: _applicationSettings(applicationSettings)
{
	assert(_applicationSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageExternalProgramsId});
	SetPageTitle("External Programs");
	SetWidgetFactory([this](EditorContext* editorContext)
		{
			return new OptionsPageExternalProgramsWidget(editorContext, _applicationSettings.get());
		});
}

OptionsPageExternalPrograms::~OptionsPageExternalPrograms() = default;

OptionsPageExternalProgramsWidget::OptionsPageExternalProgramsWidget(
	EditorContext* editorContext, ApplicationSettings* applicationSettings)
	: _editorContext(editorContext)
	, _applicationSettings(applicationSettings)
{
	_ui.setupUi(this);

	_ui.Compiler->setText(_applicationSettings->GetStudiomdlCompilerFileName());
	_ui.Decompiler->setText(_applicationSettings->GetStudiomdlDecompilerFileName());
	_ui.XashModelViewer->setText(_applicationSettings->GetXashModelViewerFileName());
	_ui.Quake1ModelViewer->setText(_applicationSettings->GetQuake1ModelViewerFileName());
	_ui.Source1ModelViewer->setText(_applicationSettings->GetSource1ModelViewerFileName());

	connect(_ui.BrowseCompiler, &QPushButton::clicked, this, &OptionsPageExternalProgramsWidget::OnBrowseCompiler);
	connect(_ui.BrowseDecompiler, &QPushButton::clicked, this, &OptionsPageExternalProgramsWidget::OnBrowseDecompiler);
	connect(_ui.BrowseXashModelViewer, &QPushButton::clicked,
		this, &OptionsPageExternalProgramsWidget::OnBrowseXashModelViewer);
	connect(_ui.BrowseQuake1ModelViewer, &QPushButton::clicked,
		this, &OptionsPageExternalProgramsWidget::OnBrowseQuake1ModelViewer);
	connect(_ui.BrowseSource1ModelViewer, &QPushButton::clicked,
		this, &OptionsPageExternalProgramsWidget::OnBrowseSource1ModelViewer);
}

OptionsPageExternalProgramsWidget::~OptionsPageExternalProgramsWidget() = default;

void OptionsPageExternalProgramsWidget::ApplyChanges(QSettings& settings)
{
	_applicationSettings->SetStudiomdlCompilerFileName(_ui.Compiler->text());
	_applicationSettings->SetStudiomdlDecompilerFileName(_ui.Decompiler->text());
	_applicationSettings->SetXashModelViewerFileName(_ui.XashModelViewer->text());
	_applicationSettings->SetQuake1ModelViewerFileName(_ui.Quake1ModelViewer->text());
	_applicationSettings->SetSource1ModelViewerFileName(_ui.Source1ModelViewer->text());

	_applicationSettings->SaveSettings();
}

static void BrowseExeFile(QWidget* parent, const QString& title, QLineEdit* lineEdit)
{
	const QString fileName{QFileDialog::getOpenFileName(parent, title, lineEdit->text(), ExternalProgramsExeFilter)};

	if (!fileName.isEmpty())
	{
		lineEdit->setText(fileName);
	}
}

void OptionsPageExternalProgramsWidget::OnBrowseCompiler()
{
	BrowseExeFile(this, "Select Studiomdl Compiler", _ui.Compiler);
}

void OptionsPageExternalProgramsWidget::OnBrowseDecompiler()
{
	BrowseExeFile(this, "Select Studiomdl Decompiler", _ui.Decompiler);
}

void OptionsPageExternalProgramsWidget::OnBrowseXashModelViewer()
{
	BrowseExeFile(this, "Select Xash Model Viewer", _ui.XashModelViewer);
}

void OptionsPageExternalProgramsWidget::OnBrowseQuake1ModelViewer()
{
	BrowseExeFile(this, "Select Quake 1 Model Viewer", _ui.Quake1ModelViewer);
}

void OptionsPageExternalProgramsWidget::OnBrowseSource1ModelViewer()
{
	BrowseExeFile(this, "Select Source 1 Model Viewer", _ui.Source1ModelViewer);
}
