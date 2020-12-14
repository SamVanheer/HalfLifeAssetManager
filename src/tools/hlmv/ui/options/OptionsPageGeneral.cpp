#include <cassert>

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/settings/GeneralSettings.hpp"

namespace ui::options
{
const QString OptionsPageGeneralCategory{QStringLiteral("A.General")};
const QString OptionsPageGeneralId{QStringLiteral("A.General")};

OptionsPageGeneral::OptionsPageGeneral(std::unique_ptr<settings::GeneralSettings>&& generalSettings)
	: _generalSettings(std::move(generalSettings))
{
	assert(_generalSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageGeneralId});
	SetPageTitle("General");
	SetWidgetFactory([this](EditorContext* editorContext) { return new OptionsPageGeneralWidget(editorContext, _generalSettings.get()); });
}

OptionsPageGeneral::~OptionsPageGeneral() = default;

OptionsPageGeneralWidget::OptionsPageGeneralWidget(EditorContext* editorContext, settings::GeneralSettings* generalSettings, QWidget* parent)
	: OptionsWidget(parent)
	, _editorContext(editorContext)
	, _generalSettings(generalSettings)
{
	_ui.setupUi(this);

	_ui.UseSingleInstance->setChecked(_generalSettings->ShouldUseSingleInstance());

	_ui.FloorLengthSlider->setRange(_editorContext->MinimumFloorLength, _editorContext->MaximumFloorLength);
	_ui.FloorLengthSpinner->setRange(_editorContext->MinimumFloorLength, _editorContext->MaximumFloorLength);

	_ui.FloorLengthSlider->setValue(_editorContext->GetFloorLength());
	_ui.FloorLengthSpinner->setValue(_editorContext->GetFloorLength());

	connect(_ui.FloorLengthSlider, &QSlider::valueChanged, _ui.FloorLengthSpinner, &QSpinBox::setValue);
	connect(_ui.FloorLengthSpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.FloorLengthSlider, &QSlider::setValue);
	connect(_ui.ResetFloorLength, &QPushButton::clicked, this, &OptionsPageGeneralWidget::OnResetFloorLength);
}

OptionsPageGeneralWidget::~OptionsPageGeneralWidget() = default;

void OptionsPageGeneralWidget::ApplyChanges(QSettings& settings)
{
	_generalSettings->SetUseSingleInstance(_ui.UseSingleInstance->isChecked());
	_editorContext->SetFloorLength(_ui.FloorLengthSlider->value());

	//TODO: save to settings object
	_generalSettings->SaveSettings(settings);
}

void OptionsPageGeneralWidget::OnResetFloorLength()
{
	_ui.FloorLengthSlider->setValue(_editorContext->DefaultFloorLength);
	_ui.FloorLengthSpinner->setValue(_editorContext->DefaultFloorLength);
}
}
