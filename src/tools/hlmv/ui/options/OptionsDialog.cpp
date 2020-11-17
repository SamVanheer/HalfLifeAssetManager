#include "ui/options/OptionsDialog.hpp"
#include "ui/options/OptionsPageGeneral.hpp"

namespace ui::options
{
OptionsDialog::OptionsDialog(QWidget* parent)
	: QDialog(parent)
{
	_ui.setupUi(this);

	_pageGeneral = new OptionsPageGeneral(this);

	_ui.OptionsPages->addTab(_pageGeneral, "General");

	connect(_ui.DialogButtons, &QDialogButtonBox::clicked, this, &OptionsDialog::OnButtonClicked);

	connect(this, &OptionsDialog::SaveChanges, _pageGeneral, &OptionsPageGeneral::OnSaveChanges);
}

OptionsDialog::~OptionsDialog() = default;

void OptionsDialog::OnButtonClicked(QAbstractButton* button)
{
	const auto role = _ui.DialogButtons->buttonRole(button);

	if (role == QDialogButtonBox::ButtonRole::AcceptRole ||
		role == QDialogButtonBox::ButtonRole::ApplyRole)
	{
		QSettings settings;

		SaveChanges(settings);
	}
}
}
