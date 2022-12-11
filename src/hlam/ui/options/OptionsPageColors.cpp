#include <cassert>

#include <QColorDialog>

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageColors.hpp"
#include "ui/options/OptionsPageGeneral.hpp"
#include "ui/settings/ColorSettings.hpp"

namespace ui::options
{
const QString OptionsPageColorsId{QStringLiteral("D.Colors")};

OptionsPageColors::OptionsPageColors(
	const std::shared_ptr<settings::ColorSettings>& colorSettings)
	: _colorSettings(colorSettings)
{
	assert(_colorSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageColorsId});
	SetPageTitle("Colors");
	SetWidgetFactory([this](EditorContext* editorContext)
		{
			return new OptionsPageColorsWidget(editorContext, _colorSettings.get());
		});
}

OptionsPageColors::~OptionsPageColors() = default;

OptionsPageColorsWidget::OptionsPageColorsWidget(
	EditorContext* editorContext, settings::ColorSettings* colorSettings)
	: _editorContext(editorContext)
	, _colorSettings(colorSettings)
{
	_ui.setupUi(this);

	connect(_ui.ColorList, qOverload<int>(&QComboBox::currentIndexChanged), this, &OptionsPageColorsWidget::OnKeyChanged);
	connect(_ui.ChangeColor, &QPushButton::clicked, this, &OptionsPageColorsWidget::OnChangeColor);
	connect(_ui.Default, &QPushButton::clicked, this, &OptionsPageColorsWidget::OnResetColor);

	auto keys{_colorSettings->GetKeys()};

	keys.sort(Qt::CaseSensitivity::CaseInsensitive);

	for (const auto& key : keys)
	{
		_ui.ColorList->addItem(key, _colorSettings->GetColor(key));
	}
}

OptionsPageColorsWidget::~OptionsPageColorsWidget() = default;

void OptionsPageColorsWidget::ApplyChanges(QSettings& settings)
{
	for (int i = 0; i < _ui.ColorList->count(); ++i)
	{
		_colorSettings->Set(_ui.ColorList->itemText(i), _ui.ColorList->itemData(i).value<QColor>());
	}

	emit _colorSettings->ColorsChanged();

	_colorSettings->SaveSettings(settings);
}

void OptionsPageColorsWidget::SetPreviewColor(const QColor& color)
{
	_ui.Preview->setStyleSheet(QString{"border: 1px solid black; background-color: %1"}.arg(color.name()));
}

void OptionsPageColorsWidget::OnKeyChanged(int index)
{
	const QColor color{_ui.ColorList->itemData(index).value<QColor>()};

	SetPreviewColor(color);
}

void OptionsPageColorsWidget::OnChangeColor()
{
	const QColor current{_ui.ColorList->currentData().value<QColor>()};

	const QColor color{QColorDialog::getColor(current, this)};

	if (color.isValid())
	{
		_colorSettings->Set(_ui.ColorList->currentText(), color);
		_ui.ColorList->setItemData(_ui.ColorList->currentIndex(), color);
		SetPreviewColor(color);
	}
}

void OptionsPageColorsWidget::OnResetColor()
{
	const QString key{_ui.ColorList->currentText()};

	const QColor defaultColor{_colorSettings->GetDefaultColor(key)};

	_colorSettings->Set(key, defaultColor);
	_ui.ColorList->setItemData(_ui.ColorList->currentIndex(), defaultColor);
	SetPreviewColor(defaultColor);
}
}
