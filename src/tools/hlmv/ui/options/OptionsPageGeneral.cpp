#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageGeneral.hpp"

namespace ui::options
{
const QString OptionsPageGeneralId{QStringLiteral("A.General")};

OptionsPageGeneral::OptionsPageGeneral()
{
	SetId(QString{OptionsPageGeneralId});
	SetTitle("General");
	SetWidgetFactory([](EditorContext* editorContext) { return new OptionsPageGeneralWidget(editorContext); });
}

OptionsPageGeneralWidget::OptionsPageGeneralWidget(EditorContext* editorContext, QWidget* parent)
	: OptionsWidget(parent)
	, _editorContext(editorContext)
{
	_ui.setupUi(this);

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
	_editorContext->SetFloorLength(_ui.FloorLengthSlider->value());

	//TODO: save to settings object
}

void OptionsPageGeneralWidget::OnSaveChanges(QSettings& settings)
{
	_editorContext->SetFloorLength(_ui.FloorLengthSlider->value());

	//TODO: save to settings object
}

void OptionsPageGeneralWidget::OnResetFloorLength()
{
	_ui.FloorLengthSlider->setValue(_editorContext->DefaultFloorLength);
	_ui.FloorLengthSpinner->setValue(_editorContext->DefaultFloorLength);
}
}
