#include <cassert>

#include <QColorDialog>

#include "settings/ColorSettings.hpp"

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageColors.hpp"
#include "ui/options/OptionsPageGeneral.hpp"

const QString OptionsPageColorsId{QStringLiteral("D.Colors")};

OptionsPageColors::OptionsPageColors(
	const std::shared_ptr<ColorSettings>& colorSettings)
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
	EditorContext* editorContext, ColorSettings* colorSettings)
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
		_ui.ColorList->addItem(key, VectorToColor(_colorSettings->GetColor(key)));
	}
}

OptionsPageColorsWidget::~OptionsPageColorsWidget() = default;

void OptionsPageColorsWidget::ApplyChanges(QSettings& settings)
{
	for (int i = 0; i < _ui.ColorList->count(); ++i)
	{
		_colorSettings->Set(_ui.ColorList->itemText(i), ColorToVector(_ui.ColorList->itemData(i).value<QColor>()));
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

	QColorDialog::ColorDialogOptions options{};

	if (_colorSettings->HasAlphaChannel(_ui.ColorList->currentText()))
	{
		options |= QColorDialog::ColorDialogOption::ShowAlphaChannel;
	}

	const QColor color{QColorDialog::getColor(current, this, {}, options)};

	if (color.isValid())
	{
		_ui.ColorList->setItemData(_ui.ColorList->currentIndex(), color);
		SetPreviewColor(color);
	}
}

void OptionsPageColorsWidget::OnResetColor()
{
	const QString key{_ui.ColorList->currentText()};

	const glm::vec4 defaultColor{_colorSettings->GetDefaultColor(key)};

	const QColor defaultColorAsColor = VectorToColor(defaultColor);

	_ui.ColorList->setItemData(_ui.ColorList->currentIndex(), defaultColorAsColor);
	SetPreviewColor(defaultColorAsColor);
}
