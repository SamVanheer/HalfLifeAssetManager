#include <cassert>

#include <QFileDialog>

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/options/OptionsPageStyle.hpp"

#include "ui/settings/StyleSettings.hpp"

namespace ui::options
{
const QString OptionsPageStyleId{QStringLiteral("B.Style")};

OptionsPageStyle::OptionsPageStyle(const std::shared_ptr<settings::StyleSettings>& styleSettings)
	: _styleSettings(styleSettings)
{
	assert(_styleSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageStyleId});
	SetPageTitle("Style");
	SetWidgetFactory([this](EditorContext* editorContext)
		{
			return new OptionsPageStyleWidget(editorContext, _styleSettings.get());
		});
}

OptionsPageStyle::~OptionsPageStyle() = default;

OptionsPageStyleWidget::OptionsPageStyleWidget(EditorContext* editorContext, settings::StyleSettings* styleSettings, QWidget* parent)
	: OptionsWidget(parent)
	, _editorContext(editorContext)
	, _styleSettings(styleSettings)
{
	_ui.setupUi(this);

	_ui.CurrentStyle->setText(_styleSettings->GetStylePath());

	connect(_ui.BrowseStyle, &QPushButton::clicked, this, &OptionsPageStyleWidget::OnBrowseStyle);
}

OptionsPageStyleWidget::~OptionsPageStyleWidget() = default;

void OptionsPageStyleWidget::ApplyChanges(QSettings& settings)
{
	_styleSettings->SetStylePath(_ui.CurrentStyle->text());

	_styleSettings->SaveSettings(settings);
}

void OptionsPageStyleWidget::OnBrowseStyle()
{
	const QString fileName{QFileDialog::getOpenFileName(this, "Select style sheet", _ui.CurrentStyle->text(), "Qt Style Sheets (*.qss);;All Files (*.*)")};

	if (!fileName.isEmpty())
	{
		_ui.CurrentStyle->setText(fileName);
	}
}
}
