#include <cassert>

#include "settings/ApplicationSettings.hpp"
#include "settings/RecentFilesSettings.hpp"

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageGeneral.hpp"

const QString OptionsPageGeneralCategory{QStringLiteral("A.General")};
const QString OptionsPageGeneralId{QStringLiteral("A.General")};

OptionsPageGeneral::OptionsPageGeneral(const std::shared_ptr<ApplicationSettings>& applicationSettings,
	const std::shared_ptr<RecentFilesSettings>& recentFilesSettings)
	: _applicationSettings(applicationSettings)
	, _recentFilesSettings(recentFilesSettings)
{
	assert(applicationSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageGeneralId});
	SetPageTitle("General");
	SetWidgetFactory([this](EditorContext* editorContext)
		{
			return new OptionsPageGeneralWidget(editorContext, _applicationSettings.get(), _recentFilesSettings.get());
		});
}

OptionsPageGeneral::~OptionsPageGeneral() = default;

OptionsPageGeneralWidget::OptionsPageGeneralWidget(
	EditorContext* editorContext, ApplicationSettings* applicationSettings, RecentFilesSettings* recentFilesSettings)
	: _editorContext(editorContext)
	, _applicationSettings(applicationSettings)
	, _recentFilesSettings(recentFilesSettings)
{
	_ui.setupUi(this);

	auto settings = _editorContext->GetSettings();

	_ui.TickRate->setRange(ApplicationSettings::MinimumTickRate, ApplicationSettings::MaximumTickRate);

	_ui.MouseSensitivitySlider->setRange(ApplicationSettings::MinimumMouseSensitivity, ApplicationSettings::MaximumMouseSensitivity);
	_ui.MouseSensitivitySpinner->setRange(ApplicationSettings::MinimumMouseSensitivity, ApplicationSettings::MaximumMouseSensitivity);

	_ui.MouseWheelSpeedSlider->setRange(ApplicationSettings::MinimumMouseWheelSpeed, ApplicationSettings::MaximumMouseWheelSpeed);
	_ui.MouseWheelSpeedSpinner->setRange(ApplicationSettings::MinimumMouseWheelSpeed, ApplicationSettings::MaximumMouseWheelSpeed);

	_ui.UseSingleInstance->setChecked(_applicationSettings->ShouldUseSingleInstance());
	_ui.PauseAnimationsOnTimelineClick->setChecked(_applicationSettings->PauseAnimationsOnTimelineClick);
	_ui.AllowTabCloseWithMiddleClick->setChecked(
		settings->value("General/AllowTabCloseWithMiddleClick", ApplicationSettings::DefaultAllowTabCloseWithMiddleClick).toBool());
	_ui.OneAssetAtATime->setChecked(_applicationSettings->OneAssetAtATime);
	_ui.PromptExternalProgramLaunch->setChecked(_applicationSettings->PromptExternalProgramLaunch);
	_ui.MaxRecentFiles->setValue(_recentFilesSettings->GetMaxRecentFiles());
	_ui.TickRate->setValue(_applicationSettings->GetTickRate());
	_ui.InvertMouseX->setChecked(_applicationSettings->ShouldInvertMouseX());
	_ui.InvertMouseY->setChecked(_applicationSettings->ShouldInvertMouseY());
	_ui.MouseSensitivitySlider->setValue(_applicationSettings->GetMouseSensitivity());
	_ui.MouseSensitivitySpinner->setValue(_applicationSettings->GetMouseSensitivity());
	_ui.MouseWheelSpeedSlider->setValue(_applicationSettings->GetMouseWheelSpeed());
	_ui.MouseWheelSpeedSpinner->setValue(_applicationSettings->GetMouseWheelSpeed());
	_ui.EnableAudioPlayback->setChecked(_applicationSettings->ShouldEnableAudioPlayback());
	_ui.EnableVerticalSync->setChecked(ApplicationSettings::ShouldEnableVSync(*settings));

	connect(_ui.MouseSensitivitySlider, &QSlider::valueChanged, _ui.MouseSensitivitySpinner, &QSpinBox::setValue);
	connect(_ui.MouseSensitivitySpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.MouseSensitivitySlider, &QSlider::setValue);

	connect(_ui.MouseWheelSpeedSlider, &QSlider::valueChanged, _ui.MouseWheelSpeedSpinner, &QSpinBox::setValue);
	connect(_ui.MouseWheelSpeedSpinner, qOverload<int>(&QSpinBox::valueChanged), _ui.MouseWheelSpeedSlider, &QSlider::setValue);
}

OptionsPageGeneralWidget::~OptionsPageGeneralWidget() = default;

void OptionsPageGeneralWidget::ApplyChanges(QSettings& settings)
{
	_applicationSettings->SetUseSingleInstance(_ui.UseSingleInstance->isChecked());
	_applicationSettings->PauseAnimationsOnTimelineClick = _ui.PauseAnimationsOnTimelineClick->isChecked();
	settings.setValue("General/AllowTabCloseWithMiddleClick", _ui.AllowTabCloseWithMiddleClick->isChecked());
	_applicationSettings->OneAssetAtATime = _ui.OneAssetAtATime->isChecked();
	_applicationSettings->PromptExternalProgramLaunch = _ui.PromptExternalProgramLaunch->isChecked();
	_recentFilesSettings->SetMaxRecentFiles(_ui.MaxRecentFiles->value());
	_applicationSettings->SetTickRate(_ui.TickRate->value());
	_applicationSettings->SetInvertMouseX(_ui.InvertMouseX->isChecked());
	_applicationSettings->SetInvertMouseY(_ui.InvertMouseY->isChecked());
	_applicationSettings->SetMouseSensitivity(_ui.MouseSensitivitySlider->value());
	_applicationSettings->SetMouseWheelSpeed(_ui.MouseWheelSpeedSlider->value());
	_applicationSettings->SetEnableAudioPlayback(_ui.EnableAudioPlayback->isChecked());
	_applicationSettings->SetEnableVSync(_ui.EnableVerticalSync->isChecked());

	_applicationSettings->SaveSettings(settings);
	_recentFilesSettings->SaveSettings(settings);
}
