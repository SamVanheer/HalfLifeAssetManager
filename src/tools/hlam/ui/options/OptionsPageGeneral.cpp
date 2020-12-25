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

	_ui.UseSingleInstance->setChecked(_generalSettings->ShouldUseSingleInstance());
	_ui.MaxRecentFiles->setValue(_recentFilesSettings->GetMaxRecentFiles());
	_ui.MaxFPS->setValue(static_cast<int>(_generalSettings->GetMaxFPS()));
	_ui.InvertMouseX->setChecked(_generalSettings->ShouldInvertMouseX());
	_ui.InvertMouseY->setChecked(_generalSettings->ShouldInvertMouseY());
}

OptionsPageGeneralWidget::~OptionsPageGeneralWidget() = default;

void OptionsPageGeneralWidget::ApplyChanges(QSettings& settings)
{
	_generalSettings->SetUseSingleInstance(_ui.UseSingleInstance->isChecked());
	_recentFilesSettings->SetMaxRecentFiles(_ui.MaxRecentFiles->value());
	_generalSettings->SetMaxFPS(_ui.MaxFPS->value());
	_generalSettings->SetInvertMouseX(_ui.InvertMouseX->isChecked());
	_generalSettings->SetInvertMouseY(_ui.InvertMouseY->isChecked());

	_generalSettings->SaveSettings(settings);
	_recentFilesSettings->SaveSettings(settings);
}
}
