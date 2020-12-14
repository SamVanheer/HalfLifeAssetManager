#include "ui/EditorContext.hpp"

#include "ui/options/OptionsDialog.hpp"
#include "ui/options/OptionsPage.hpp"
#include "ui/options/OptionsPageRegistry.hpp"

namespace ui::options
{
OptionsDialog::OptionsDialog(EditorContext* editorContext, QWidget* parent)
	: QDialog(parent)
	, _editorContext(editorContext)
{
	_ui.setupUi(this);

	_pages = _editorContext->GetOptionsPageRegistry()->GetPages();

	for (auto page : _pages)
	{
		_ui.OptionsPages->addTab(page->GetWidget(_editorContext), page->GetTitle());
	}

	connect(_ui.DialogButtons, &QDialogButtonBox::clicked, this, &OptionsDialog::OnButtonClicked);
}

OptionsDialog::~OptionsDialog()
{
	for (auto page : _pages)
	{
		page->DestroyWidget();
	}
}

void OptionsDialog::OnButtonClicked(QAbstractButton* button)
{
	const auto role = _ui.DialogButtons->buttonRole(button);

	if (role == QDialogButtonBox::ButtonRole::AcceptRole ||
		role == QDialogButtonBox::ButtonRole::ApplyRole)
	{
		auto settings = _editorContext->GetSettings();

		for (auto page : _pages)
		{
			page->ApplyChanges(*settings);
		}

		settings->sync();
	}
}
}
