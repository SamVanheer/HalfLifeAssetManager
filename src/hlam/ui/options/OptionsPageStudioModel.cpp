#include <cassert>

#include <QFileDialog>

#include "settings/StudioModelSettings.hpp"

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"

const QString OptionsPageStudioModelCategory{QStringLiteral("D.Assets")};
const QString OptionsPageStudioModelId{QStringLiteral("Studiomodel")};

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

	connect(_ui.FloorLengthSlider, &QSlider::valueChanged, _ui.FloorLengthSpinner, &QSpinBox::setValue);
	connect(_ui.FloorLengthSpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.FloorLengthSlider, &QSlider::setValue);
	connect(_ui.ResetFloorLength, &QPushButton::clicked, this, &OptionsPageStudioModelWidget::OnResetFloorLength);
}

OptionsPageStudioModelWidget::~OptionsPageStudioModelWidget() = default;

void OptionsPageStudioModelWidget::ApplyChanges()
{
	_studioModelSettings->SetAutodetectViewmodels(_ui.AutodetectViewmodels->isChecked());
	_studioModelSettings->SetActivateTextureViewWhenTexturesPanelOpened(_ui.ActivateTextureViewWhenTexturesPanelOpened->isChecked());
	_studioModelSettings->SetFloorLength(_ui.FloorLengthSlider->value());
}

void OptionsPageStudioModelWidget::OnResetFloorLength()
{
	_ui.FloorLengthSlider->setValue(_studioModelSettings->DefaultFloorLength);
	_ui.FloorLengthSpinner->setValue(_studioModelSettings->DefaultFloorLength);
}
