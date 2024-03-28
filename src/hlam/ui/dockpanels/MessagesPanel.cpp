#include <cstring>

#include <QScrollBar>

#include "application/AssetManager.hpp"

#include "ui_MessagesPanel.h"

#include "ui/dockpanels/MessagesPanel.hpp"

const int MaxLines = 1000;

MessagesPanel::MessagesPanel(AssetManager* application, QWidget* parent)
	: QWidget(parent)
	, _ui(std::make_unique<Ui_MessagesPanel>())
	, _application(application)
{
	_ui->setupUi(this);

	_defaultTextColor = _ui->Text->textColor();

	connect(_ui->Clear, &QPushButton::clicked, _ui->Text, &QTextEdit::clear);
	connect(_application, &AssetManager::LogMessageReceived, this, &MessagesPanel::OnMessageReceived);
}

MessagesPanel::~MessagesPanel() = default;

void MessagesPanel::OnMessageReceived(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	if (std::strcmp(context.category, "default") == 0 && !_ui->QtDiagnostics->isChecked())
	{
		return;
	}

	if (type == QtDebugMsg && !_ui->Debug->isChecked())
	{
		return;
	}

	if (type == QtInfoMsg && !_ui->Info->isChecked())
	{
		return;
	}

	const bool isCritical = type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg;

	_ui->Text->setTextColor(isCritical ? Qt::GlobalColor::red : _defaultTextColor);

	if (!_ui->Text->document()->isEmpty())
	{
		_ui->Text->insertPlainText("\n");
	}

	if (_ui->Category->isChecked())
	{
		_ui->Text->insertPlainText(QString{"%1: "}.arg(context.category));
	}

	_ui->Text->insertPlainText(msg);

	if (int lines = _ui->Text->document()->lineCount(); lines > MaxLines)
	{
		QTextCursor cursor = _ui->Text->textCursor();

		do
		{
			cursor.movePosition(QTextCursor::Start);
			cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 0);
			cursor.select(QTextCursor::LineUnderCursor);
			cursor.removeSelectedText();
			cursor.deleteChar(); // clean up new line
		} while (--lines > MaxLines);

		_ui->Text->setTextCursor(cursor);
	}

	_ui->Text->verticalScrollBar()->setValue(_ui->Text->verticalScrollBar()->maximum());

	if (isCritical)
	{
		parentWidget()->show();
	}
}
