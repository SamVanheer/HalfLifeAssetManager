#include <cassert>

#include <QFileDialog>
#include <QPushButton>

#include "settings/ApplicationSettings.hpp"

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageExternalPrograms.hpp"
#include "ui/options/OptionsPageGeneral.hpp"

const QString OptionsPageExternalProgramsId{QStringLiteral("E.ExternalPrograms")};
const QString ExternalProgramsExeFilter{QStringLiteral("Executable Files (*.exe *.com);;All Files (*.*)")};

OptionsPageExternalPrograms::OptionsPageExternalPrograms(const std::shared_ptr<ApplicationSettings>& applicationSettings)
	: _applicationSettings(applicationSettings)
{
	assert(_applicationSettings);

	SetCategory(QString{OptionsPageGeneralCategory});
	SetCategoryTitle("General");
	SetId(QString{OptionsPageExternalProgramsId});
	SetPageTitle("External Programs");
	SetWidgetFactory([this](EditorContext* editorContext)
		{
			return new OptionsPageExternalProgramsWidget(editorContext, _applicationSettings.get());
		});
}

OptionsPageExternalPrograms::~OptionsPageExternalPrograms() = default;

OptionsPageExternalProgramsWidget::OptionsPageExternalProgramsWidget(
	EditorContext* editorContext, ApplicationSettings* applicationSettings)
	: _editorContext(editorContext)
	, _applicationSettings(applicationSettings)
{
	_ui.setupUi(this);

	auto externalPrograms = _applicationSettings->GetExternalPrograms();

	auto keys = externalPrograms->GetMap().keys();

	keys.sort();

	_ui.Programs->setRowCount(keys.count());

	for (int row = 0; const auto& key : keys)
	{
		auto name = new QTableWidgetItem(externalPrograms->GetName(key));

		name->setFlags(name->flags() & ~(Qt::ItemFlag::ItemIsEditable));

		_ui.Programs->setItem(row, 0, name);

		auto executable = new QTableWidgetItem(externalPrograms->GetProgram(key));

		executable->setData(Qt::UserRole, key);

		_ui.Programs->setItem(row, 1, executable);

		auto button = new QPushButton("Browse", _ui.Programs);

		button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		connect(button, &QPushButton::clicked,
			this, [this, name, executable]()
			{
				const QString fileName{QFileDialog::getOpenFileName(
					this, QString{"Select %1"}.arg(name->text()), executable->text(), ExternalProgramsExeFilter)};

				if (!fileName.isEmpty())
				{
					executable->setText(fileName);
				}
			});

		_ui.Programs->setCellWidget(row, 2, button);

		++row;
	}

	_ui.Programs->resizeColumnsToContents();

	_ui.Programs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
	_ui.Programs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	_ui.Programs->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Fixed);

	setEnabled(keys.size() > 0);
}

OptionsPageExternalProgramsWidget::~OptionsPageExternalProgramsWidget() = default;

void OptionsPageExternalProgramsWidget::ApplyChanges()
{
	auto externalPrograms = _applicationSettings->GetExternalPrograms();

	for (int row = 0; row < _ui.Programs->rowCount(); ++row)
	{
		auto executable = _ui.Programs->item(row, 1);

		externalPrograms->SetProgram(executable->data(Qt::UserRole).toString(), executable->text());
	}
}
