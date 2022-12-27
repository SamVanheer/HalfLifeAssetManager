#include <cassert>

#include <QFileDialog>

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/settings/GeneralSettings.hpp"

const QString OptionsPageExternalProgramsId{QStringLiteral("E.ExternalPrograms")};
const QString ExternalProgramsExeFilter{QStringLiteral("Executable Files (*.exe *.com);;All Files (*.*)")};

OptionsPageExternalPrograms::OptionsPageExternalPrograms(const std::shared_ptr<GeneralSettings>& generalSettings)
	: _generalSettings(generalSettings)
{
	assert(_generalSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageExternalProgramsId});
	SetPageTitle("External Programs");
	SetWidgetFactory([this](EditorContext* editorContext) { return new OptionsPageExternalProgramsWidget(editorContext, _generalSettings.get()); });
}

OptionsPageExternalPrograms::~OptionsPageExternalPrograms() = default;

OptionsPageExternalProgramsWidget::OptionsPageExternalProgramsWidget(EditorContext* editorContext, GeneralSettings* generalSettings)
	: _editorContext(editorContext)
	, _generalSettings(generalSettings)
{
	_ui.setupUi(this);

	_ui.Compiler->setText(_generalSettings->GetStudiomdlCompilerFileName());
	_ui.Decompiler->setText(_generalSettings->GetStudiomdlDecompilerFileName());
	_ui.XashModelViewer->setText(_generalSettings->GetXashModelViewerFileName());
	_ui.Quake1ModelViewer->setText(_generalSettings->GetQuake1ModelViewerFileName());
	_ui.Source1ModelViewer->setText(_generalSettings->GetSource1ModelViewerFileName());

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
	_generalSettings->SetStudiomdlCompilerFileName(_ui.Compiler->text());
	_generalSettings->SetStudiomdlDecompilerFileName(_ui.Decompiler->text());
	_generalSettings->SetXashModelViewerFileName(_ui.XashModelViewer->text());
	_generalSettings->SetQuake1ModelViewerFileName(_ui.Quake1ModelViewer->text());
	_generalSettings->SetSource1ModelViewerFileName(_ui.Source1ModelViewer->text());

	_generalSettings->SaveSettings(settings);
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
