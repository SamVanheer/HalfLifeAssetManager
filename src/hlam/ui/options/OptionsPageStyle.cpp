#include <cassert>

#include <QFileDialog>

#include "settings/ApplicationSettings.hpp"

#include "application/EditorContext.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/options/OptionsPageStyle.hpp"

const QString OptionsPageStyleId{QStringLiteral("B.Style")};

OptionsPageStyle::OptionsPageStyle(const std::shared_ptr<ApplicationSettings>& applicationSettings)
	: _applicationSettings(applicationSettings)
{
	assert(_applicationSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageStyleId});
	SetPageTitle("Style");
	SetWidgetFactory([this](EditorContext* editorContext)
		{
			return new OptionsPageStyleWidget(editorContext, _applicationSettings.get());
		});
}

OptionsPageStyle::~OptionsPageStyle() = default;

OptionsPageStyleWidget::OptionsPageStyleWidget(EditorContext* editorContext, ApplicationSettings* applicationSettings)
	: _editorContext(editorContext)
	, _applicationSettings(applicationSettings)
{
	_ui.setupUi(this);

	_ui.CurrentStyle->setText(_applicationSettings->GetStylePath());

	connect(_ui.BrowseStyle, &QPushButton::clicked, this, &OptionsPageStyleWidget::OnBrowseStyle);
}

OptionsPageStyleWidget::~OptionsPageStyleWidget() = default;

void OptionsPageStyleWidget::ApplyChanges()
{
	_applicationSettings->SetStylePath(_ui.CurrentStyle->text());
}

void OptionsPageStyleWidget::OnBrowseStyle()
{
	const QString fileName{QFileDialog::getOpenFileName(this, "Select style sheet", _ui.CurrentStyle->text(), "Qt Style Sheets (*.qss);;All Files (*.*)")};

	if (!fileName.isEmpty())
	{
		_ui.CurrentStyle->setText(fileName);
	}
}
