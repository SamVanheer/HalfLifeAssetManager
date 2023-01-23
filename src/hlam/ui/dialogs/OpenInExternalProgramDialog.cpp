#include <QPainter>
#include <QStyledItemDelegate>

#include "ui_OpenInExternalProgramDialog.h"

#include "application/AssetManager.hpp"
#include "settings/ApplicationSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"
#include "settings/RecentFilesSettings.hpp"

#include "ui/dialogs/OpenInExternalProgramDialog.hpp"

/**
*	@brief Elides the text in table cells on the left side so the most significant part of the filename is still visible.
*/
class ElidingItemDelegate final : public QStyledItemDelegate
{
public:
	using QStyledItemDelegate::QStyledItemDelegate;

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		if (!index.isValid())
			return;

		QStyleOptionViewItem textOpt = option;
		initStyleOption(&textOpt, index);

		// Draw a gray background if it's disabled.
		// We can't rely on style flags here because disabled items
		// are drawn the same way as enabled items on some platforms.
		{
			QStyleOptionViewItem backgroundOpt = textOpt;

			backgroundOpt.rect = option.rect;
			backgroundOpt.textElideMode = Qt::ElideNone;
			backgroundOpt.text = "";

			if ((index.flags() & Qt::ItemIsEnabled) == 0)
			{
				backgroundOpt.backgroundBrush = QBrush{Qt::gray};
			}

			QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &backgroundOpt, painter);
		}

		painter->save();
		painter->setClipRect(textOpt.rect);
		painter->drawText(textOpt.rect, Qt::AlignLeft | Qt::AlignVCenter, 
			textOpt.fontMetrics.elidedText(textOpt.text, Qt::ElideLeft, textOpt.rect.width()));
		painter->restore();
	}
};

OpenInExternalProgramDialog::OpenInExternalProgramDialog(AssetManager* application, QWidget* parent,
	const std::vector<ExternalProgramCommand>& commands)
	: QDialog(parent)
	, _ui(std::make_unique<Ui_OpenInExternalProgramDialog>())
	, _application(application)
{
	_ui->setupUi(this);

	const auto externalPrograms = _application->GetApplicationSettings()->GetExternalPrograms();

	_ui->FileList->setItemDelegate(new ElidingItemDelegate(this));
	_ui->FileList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	_ui->FileList->setRowCount(commands.size());

	int firstEnabledItem = -1;

	for (int row = 0; const auto& command : commands)
	{
		auto item = new QTableWidgetItem(command.FileName);
		item->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		item->setData(Qt::UserRole, command.ExternalProgramKey);

		auto name = new QTableWidgetItem(externalPrograms->GetName(command.ExternalProgramKey));
		name->setFlags(name->flags() & ~(Qt::ItemIsEditable));

		// Disable files that have no program configured for them.
		// Don't check if the program exists, TryLaunchExternalProgram does that already.
		if (externalPrograms->GetProgram(command.ExternalProgramKey).isEmpty())
		{
			item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
			name->setFlags(name->flags() & ~Qt::ItemIsEnabled);
		}
		else if (firstEnabledItem == -1)
		{
			firstEnabledItem = row;
		}

		_ui->FileList->setItem(row, 0, item);
		_ui->FileList->setItem(row, 1, name);

		++row;
	}

	_ui->FileList->resizeColumnToContents(1);
	_ui->FileList->selectRow(firstEnabledItem);

	if (firstEnabledItem == -1)
	{
		_ui->Open->setEnabled(false);
		_ui->OpenAll->setEnabled(false);
	}

	connect(_ui->FileList, &QTableWidget::itemActivated, this, &OpenInExternalProgramDialog::OpenSelectedFile);
	connect(_ui->Open, &QPushButton::clicked, this, &OpenInExternalProgramDialog::OpenSelectedFile);
	connect(_ui->OpenAll, &QPushButton::clicked, this,
		[this]
		{
			for (int i = 0; i < _ui->FileList->rowCount(); ++i)
			{
				OpenRow(i);
			}
		});
}

OpenInExternalProgramDialog::~OpenInExternalProgramDialog() = default;

void OpenInExternalProgramDialog::OpenRow(int row)
{
	const auto item = _ui->FileList->item(row, 0);

	if ((item->flags() & Qt::ItemIsEnabled) == 0)
	{
		return;
	}

	const auto externalPrograms = _application->GetApplicationSettings()->GetExternalPrograms();

	const auto fileName = item->text();
	const auto externalProgramKey = item->data(Qt::UserRole).toString();

	const auto launchResult = _application->TryLaunchExternalProgram(externalProgramKey, QStringList{fileName});

	if (launchResult == LaunchExternalProgramResult::Success)
	{
		_application->GetApplicationSettings()->GetRecentFiles()->Add(fileName);
	}
}

void OpenInExternalProgramDialog::OpenSelectedFile()
{
	if (const auto selectedRows = _ui->FileList->selectionModel()->selectedRows(); !selectedRows.empty())
	{
		OpenRow(selectedRows.front().row());
	}
}
