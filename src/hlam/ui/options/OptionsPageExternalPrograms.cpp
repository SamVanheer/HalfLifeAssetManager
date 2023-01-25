#include <cassert>

#include <QFileDialog>
#include <QPushButton>
#include <QSignalBlocker>

#include "settings/ApplicationSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"

#include "application/AssetManager.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"
#include "ui/options/OptionsPageGeneral.hpp"

const QString OptionsPageExternalProgramsId{QStringLiteral("E.ExternalPrograms")};
const QString ExternalProgramsExeFilter{QStringLiteral("Executable Files (*.exe *.com);;All Files (*.*)")};

enum ExternalProgramsRole
{
	KeyRole = Qt::UserRole,
	ExecutablePathRole,
	AdditionalArgumentsRole,
};

OptionsPageExternalPrograms::OptionsPageExternalPrograms(const std::shared_ptr<ApplicationSettings>& applicationSettings)
	: _applicationSettings(applicationSettings)
{
	assert(_applicationSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageExternalProgramsId});
	SetPageTitle("External Programs");
	SetWidgetFactory([this](AssetManager* application)
		{
			return new OptionsPageExternalProgramsWidget(application, _applicationSettings.get());
		});
}

OptionsPageExternalPrograms::~OptionsPageExternalPrograms() = default;

OptionsPageExternalProgramsWidget::OptionsPageExternalProgramsWidget(
	AssetManager* application, ApplicationSettings* applicationSettings)
	: _application(application)
	, _applicationSettings(applicationSettings)
{
	_ui.setupUi(this);

	auto externalPrograms = _applicationSettings->GetExternalPrograms();

	_ui.PromptExternalProgramLaunch->setChecked(externalPrograms->PromptExternalProgramLaunch);

	auto keys = externalPrograms->GetMap().keys();

	keys.sort();

	for (int row = 0; const auto& key : keys)
	{
		_ui.Programs->addItem(externalPrograms->GetName(key));
		_ui.Programs->setItemData(row, key, KeyRole);
		_ui.Programs->setItemData(row, externalPrograms->GetProgram(key), ExecutablePathRole);
		_ui.Programs->setItemData(row, externalPrograms->GetAdditionalArguments(key), AdditionalArgumentsRole);

		++row;
	}

	connect(_ui.Programs, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &OptionsPageExternalProgramsWidget::OnProgramChanged);

	connect(_ui.ExecutablePath, &QLineEdit::textChanged, this,
		[this]
		{
			_ui.Programs->setItemData(_ui.Programs->currentIndex(), _ui.ExecutablePath->text(), ExecutablePathRole);
		});

	connect(_ui.Arguments, &QLineEdit::textChanged, this,
		[this]
		{
			_ui.Programs->setItemData(_ui.Programs->currentIndex(), _ui.Arguments->text(), AdditionalArgumentsRole);
		});

	connect(_ui.BrowseExecutablePath, &QPushButton::clicked, this,
		[this]
		{
			const QString fileName{QFileDialog::getOpenFileName(
				this, QString{"Select %1"}.arg(_ui.Programs->currentText()), _ui.ExecutablePath->text(),
				ExternalProgramsExeFilter)};

			if (!fileName.isEmpty())
			{
				_ui.ExecutablePath->setText(fileName);
			}
		});

	OnProgramChanged(_ui.Programs->currentIndex());
	setEnabled(keys.size() > 0);
}

OptionsPageExternalProgramsWidget::~OptionsPageExternalProgramsWidget() = default;

void OptionsPageExternalProgramsWidget::ApplyChanges()
{
	auto externalPrograms = _applicationSettings->GetExternalPrograms();

	externalPrograms->PromptExternalProgramLaunch = _ui.PromptExternalProgramLaunch->isChecked();

	for (int row = 0; row < _ui.Programs->count(); ++row)
	{
		const auto key = _ui.Programs->itemData(row, KeyRole).toString();

		externalPrograms->SetProgram(key, _ui.Programs->itemData(row, ExecutablePathRole).toString());
		externalPrograms->SetAdditionalArguments(key, _ui.Programs->itemData(row, AdditionalArgumentsRole).toString());
	}
}

void OptionsPageExternalProgramsWidget::OnProgramChanged(int index)
{
	const QSignalBlocker executablePathBlocker{_ui.ExecutablePath};
	const QSignalBlocker argumentsBlocker{_ui.Arguments};

	_ui.ExecutablePath->setText(_ui.Programs->itemData(index, ExecutablePathRole).toString());
	_ui.Arguments->setText(_ui.Programs->itemData(index, AdditionalArgumentsRole).toString());

	_ui.ProgramData->setEnabled(index != -1);
}
