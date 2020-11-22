#include "ui/EditorUIContext.hpp"
#include "ui/options/OptionsPageGeneral.hpp"

namespace ui::options
{
OptionsPageGeneral::OptionsPageGeneral(EditorUIContext* editorContext, QWidget* parent)
	: QWidget(parent)
	, _editorContext(editorContext)
{
	_ui.setupUi(this);

	_ui.FloorLengthSlider->setRange(_editorContext->MinimumFloorLength, _editorContext->MaximumFloorLength);
	_ui.FloorLengthSpinner->setRange(_editorContext->MinimumFloorLength, _editorContext->MaximumFloorLength);

	_ui.FloorLengthSlider->setValue(_editorContext->GetFloorLength());
	_ui.FloorLengthSpinner->setValue(_editorContext->GetFloorLength());

	connect(_ui.FloorLengthSlider, &QSlider::valueChanged, _ui.FloorLengthSpinner, &QSpinBox::setValue);
	connect(_ui.FloorLengthSpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.FloorLengthSlider, &QSlider::setValue);
	connect(_ui.ResetFloorLength, &QPushButton::clicked, this, &OptionsPageGeneral::OnResetFloorLength);
}

OptionsPageGeneral::~OptionsPageGeneral() = default;

void OptionsPageGeneral::OnSaveChanges(QSettings& settings)
{
	_editorContext->SetFloorLength(_ui.FloorLengthSlider->value());

	//TODO: save to settings object
}

void OptionsPageGeneral::OnResetFloorLength()
{
	_ui.FloorLengthSlider->setValue(_editorContext->DefaultFloorLength);
	_ui.FloorLengthSpinner->setValue(_editorContext->DefaultFloorLength);
}
}
