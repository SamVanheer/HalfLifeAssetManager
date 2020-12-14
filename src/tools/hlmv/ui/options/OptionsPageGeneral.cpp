#include <cassert>

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/settings/GeneralSettings.hpp"

namespace ui::options
{
const QString OptionsPageGeneralCategory{QStringLiteral("A.General")};
const QString OptionsPageGeneralId{QStringLiteral("A.General")};

OptionsPageGeneral::OptionsPageGeneral(const std::shared_ptr<settings::GeneralSettings>& generalSettings)
	: _generalSettings(generalSettings)
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
	_ui.InvertMouseX->setChecked(_generalSettings->ShouldInvertMouseX());
	_ui.InvertMouseY->setChecked(_generalSettings->ShouldInvertMouseY());
}

OptionsPageGeneralWidget::~OptionsPageGeneralWidget() = default;

void OptionsPageGeneralWidget::ApplyChanges(QSettings& settings)
{
	_generalSettings->SetUseSingleInstance(_ui.UseSingleInstance->isChecked());
	_generalSettings->SetInvertMouseX(_ui.InvertMouseX->isChecked());
	_generalSettings->SetInvertMouseY(_ui.InvertMouseY->isChecked());

	_generalSettings->SaveSettings(settings);
}
}
