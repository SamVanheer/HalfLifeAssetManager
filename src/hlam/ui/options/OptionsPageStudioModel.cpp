#include <cassert>

#include <QFileDialog>

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"
#include "ui/settings/StudioModelSettings.hpp"

const QString OptionsPageStudioModelCategory{QStringLiteral("D.Assets")};
const QString OptionsPageStudioModelId{QStringLiteral("Studiomodel")};
const QString StudioModelExeFilter{QStringLiteral("Executable Files (*.exe *.com);;All Files (*.*)")};

OptionsPageStudioModel::OptionsPageStudioModel(const std::shared_ptr<StudioModelSettings>& studioModelSettings)
	: _studioModelSettings(studioModelSettings)
{
	assert(_studioModelSettings);

	SetCategory(QString{OptionsPageStudioModelCategory});
	SetCategoryTitle("Assets");
	SetId(QString{OptionsPageStudioModelId});
	SetPageTitle("Studiomodel");
	SetWidgetFactory([this](EditorContext* editorContext) { return new OptionsPageStudioModelWidget(editorContext, _studioModelSettings.get()); });
}

OptionsPageStudioModel::~OptionsPageStudioModel() = default;

OptionsPageStudioModelWidget::OptionsPageStudioModelWidget(EditorContext* editorContext, StudioModelSettings* studioModelSettings)
	: _editorContext(editorContext)
	, _studioModelSettings(studioModelSettings)
{
	_ui.setupUi(this);

	_ui.AutodetectViewmodels->setChecked(_studioModelSettings->ShouldAutodetectViewmodels());
	_ui.ActivateTextureViewWhenTexturesPanelOpened->setChecked(_studioModelSettings->ShouldActivateTextureViewWhenTexturesPanelOpened());

	_ui.FloorLengthSlider->setRange(_studioModelSettings->MinimumFloorLength, _studioModelSettings->MaximumFloorLength);
	_ui.FloorLengthSpinner->setRange(_studioModelSettings->MinimumFloorLength, _studioModelSettings->MaximumFloorLength);

	_ui.FloorLengthSlider->setValue(_studioModelSettings->GetFloorLength());
	_ui.FloorLengthSpinner->setValue(_studioModelSettings->GetFloorLength());

	_ui.Compiler->setText(_studioModelSettings->GetStudiomdlCompilerFileName());
	_ui.Decompiler->setText(_studioModelSettings->GetStudiomdlDecompilerFileName());

	connect(_ui.FloorLengthSlider, &QSlider::valueChanged, _ui.FloorLengthSpinner, &QSpinBox::setValue);
	connect(_ui.FloorLengthSpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.FloorLengthSlider, &QSlider::setValue);
	connect(_ui.ResetFloorLength, &QPushButton::clicked, this, &OptionsPageStudioModelWidget::OnResetFloorLength);

	connect(_ui.BrowseCompiler, &QPushButton::clicked, this, &OptionsPageStudioModelWidget::OnBrowseCompiler);
	connect(_ui.BrowseDecompiler, &QPushButton::clicked, this, &OptionsPageStudioModelWidget::OnBrowseDecompiler);
}

OptionsPageStudioModelWidget::~OptionsPageStudioModelWidget() = default;

void OptionsPageStudioModelWidget::ApplyChanges(QSettings& settings)
{
	_studioModelSettings->SetAutodetectViewmodels(_ui.AutodetectViewmodels->isChecked());
	_studioModelSettings->SetActivateTextureViewWhenTexturesPanelOpened(_ui.ActivateTextureViewWhenTexturesPanelOpened->isChecked());
	_studioModelSettings->SetFloorLength(_ui.FloorLengthSlider->value());
	_studioModelSettings->SetStudiomdlCompilerFileName(_ui.Compiler->text());
	_studioModelSettings->SetStudiomdlDecompilerFileName(_ui.Decompiler->text());

	_studioModelSettings->SaveSettings(settings);
}

void OptionsPageStudioModelWidget::OnResetFloorLength()
{
	_ui.FloorLengthSlider->setValue(_studioModelSettings->DefaultFloorLength);
	_ui.FloorLengthSpinner->setValue(_studioModelSettings->DefaultFloorLength);
}

void OptionsPageStudioModelWidget::OnBrowseCompiler()
{
	const QString fileName{QFileDialog::getOpenFileName(this, "Select Studiomdl Compiler", _ui.Compiler->text(), StudioModelExeFilter)};

	if (!fileName.isEmpty())
	{
		_ui.Compiler->setText(fileName);
	}
}

void OptionsPageStudioModelWidget::OnBrowseDecompiler()
{
	const QString fileName{QFileDialog::getOpenFileName(this, "Select Studiomdl Decompiler", _ui.Decompiler->text(), StudioModelExeFilter)};

	if (!fileName.isEmpty())
	{
		_ui.Decompiler->setText(fileName);
	}
}
