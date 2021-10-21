#include <cassert>

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/RecentFilesSettings.hpp"

namespace ui::options
{
const QString OptionsPageGeneralCategory{QStringLiteral("A.General")};
const QString OptionsPageGeneralId{QStringLiteral("A.General")};

OptionsPageGeneral::OptionsPageGeneral(
	const std::shared_ptr<settings::GeneralSettings>& generalSettings, const std::shared_ptr<settings::RecentFilesSettings>& recentFilesSettings)
	: _generalSettings(generalSettings)
	, _recentFilesSettings(recentFilesSettings)
{
	assert(_generalSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageGeneralId});
	SetPageTitle("General");
	SetWidgetFactory([this](EditorContext* editorContext)
		{
			return new OptionsPageGeneralWidget(editorContext, _generalSettings.get(), _recentFilesSettings.get());
		});
}

OptionsPageGeneral::~OptionsPageGeneral() = default;

OptionsPageGeneralWidget::OptionsPageGeneralWidget(
	EditorContext* editorContext, settings::GeneralSettings* generalSettings, settings::RecentFilesSettings* recentFilesSettings, QWidget* parent)
	: OptionsWidget(parent)
	, _editorContext(editorContext)
	, _generalSettings(generalSettings)
	, _recentFilesSettings(recentFilesSettings)
{
	_ui.setupUi(this);

	auto settings = _editorContext->GetSettings();

	_ui.TickRate->setRange(settings::GeneralSettings::MinimumTickRate, settings::GeneralSettings::MaximumTickRate);

	_ui.MouseSensitivitySlider->setRange(settings::GeneralSettings::MinimumMouseSensitivity, settings::GeneralSettings::MaximumMouseSensitivity);
	_ui.MouseSensitivitySpinner->setRange(settings::GeneralSettings::MinimumMouseSensitivity, settings::GeneralSettings::MaximumMouseSensitivity);

	_ui.MouseWheelSpeedSlider->setRange(settings::GeneralSettings::MinimumMouseWheelSpeed, settings::GeneralSettings::MaximumMouseWheelSpeed);
	_ui.MouseWheelSpeedSpinner->setRange(settings::GeneralSettings::MinimumMouseWheelSpeed, settings::GeneralSettings::MaximumMouseWheelSpeed);

	_ui.UseSingleInstance->setChecked(_generalSettings->ShouldUseSingleInstance());
	_ui.PauseAnimationsOnTimelineClick->setChecked(_generalSettings->PauseAnimationsOnTimelineClick);
	_ui.AllowTabCloseWithMiddleClick->setChecked(
		settings->value("General/AllowTabCloseWithMiddleClick", settings::GeneralSettings::DefaultAllowTabCloseWithMiddleClick).toBool());
	_ui.MaxRecentFiles->setValue(_recentFilesSettings->GetMaxRecentFiles());
	_ui.TickRate->setValue(_generalSettings->GetTickRate());
	_ui.InvertMouseX->setChecked(_generalSettings->ShouldInvertMouseX());
	_ui.InvertMouseY->setChecked(_generalSettings->ShouldInvertMouseY());
	_ui.MouseSensitivitySlider->setValue(_generalSettings->GetMouseSensitivity());
	_ui.MouseSensitivitySpinner->setValue(_generalSettings->GetMouseSensitivity());
	_ui.MouseWheelSpeedSlider->setValue(_generalSettings->GetMouseWheelSpeed());
	_ui.MouseWheelSpeedSpinner->setValue(_generalSettings->GetMouseWheelSpeed());
	_ui.EnableAudioPlayback->setChecked(_generalSettings->ShouldEnableAudioPlayback());

	connect(_ui.MouseSensitivitySlider, &QSlider::valueChanged, _ui.MouseSensitivitySpinner, &QSpinBox::setValue);
	connect(_ui.MouseSensitivitySpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.MouseSensitivitySlider, &QSlider::setValue);

	connect(_ui.MouseWheelSpeedSlider, &QSlider::valueChanged, _ui.MouseWheelSpeedSpinner, &QSpinBox::setValue);
	connect(_ui.MouseWheelSpeedSpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.MouseWheelSpeedSlider, &QSlider::setValue);
}

OptionsPageGeneralWidget::~OptionsPageGeneralWidget() = default;

void OptionsPageGeneralWidget::ApplyChanges(QSettings& settings)
{
	_generalSettings->SetUseSingleInstance(_ui.UseSingleInstance->isChecked());
	_generalSettings->PauseAnimationsOnTimelineClick = _ui.PauseAnimationsOnTimelineClick->isChecked();
	settings.setValue("General/AllowTabCloseWithMiddleClick", _ui.AllowTabCloseWithMiddleClick->isChecked());
	_recentFilesSettings->SetMaxRecentFiles(_ui.MaxRecentFiles->value());
	_generalSettings->SetTickRate(_ui.TickRate->value());
	_generalSettings->SetInvertMouseX(_ui.InvertMouseX->isChecked());
	_generalSettings->SetInvertMouseY(_ui.InvertMouseY->isChecked());
	_generalSettings->SetMouseSensitivity(_ui.MouseSensitivitySlider->value());
	_generalSettings->SetMouseWheelSpeed(_ui.MouseWheelSpeedSlider->value());
	_generalSettings->SetEnableAudioPlayback(_ui.EnableAudioPlayback->isChecked());

	_generalSettings->SaveSettings(settings);
	_recentFilesSettings->SaveSettings(settings);
}
}
