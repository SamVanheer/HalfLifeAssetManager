#include <cmath>

#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QString>

#include "application/AssetManager.hpp"
#include "plugins/halflife/studiomodel/ui/compiler/CommandLineFrontEnd.hpp"

namespace studiomodel
{
CommandLineFrontEnd::CommandLineFrontEnd(AssetManager* application)
	: _application(application)
{
	_ui.setupUi(this);

	//Set the dialog size to a fraction of the current screen
	const QRect screenSize = QApplication::desktop()->screenGeometry(this);

	this->resize(static_cast<int>(std::ceil(screenSize.width() * 0.9)), static_cast<int>(std::ceil(screenSize.height() * 0.9)));

	_ui.RemoveFile->setEnabled(false);
	_ui.Compile->setEnabled(false);
	_ui.Terminate->setEnabled(false);

	connect(_ui.ProgramPath, &QLineEdit::textChanged, this, &CommandLineFrontEnd::UpdateCompileSettings);
	connect(_ui.BrowseProgramPath, &QPushButton::clicked, this, &CommandLineFrontEnd::OnBrowseCompiler);

	connect(_ui.AddFile, &QPushButton::clicked, this, &CommandLineFrontEnd::OnAddFile);
	connect(_ui.RemoveFile, &QPushButton::clicked, this, &CommandLineFrontEnd::OnRemoveFile);
	connect(_ui.Files, &QListWidget::currentItemChanged, this, &CommandLineFrontEnd::OnCurrentFileChanged);

	connect(_ui.BrowseWorkingDirectory, &QPushButton::clicked, this, &CommandLineFrontEnd::OnBrowseWorkingDirectory);

	connect(_ui.AdditionalCommandLineArguments, &QPlainTextEdit::textChanged, this, &CommandLineFrontEnd::UpdateCompleteCommandLine);

	connect(_ui.Compile, &QPushButton::clicked, this, &CommandLineFrontEnd::OnCompile);
	connect(_ui.Terminate, &QPushButton::clicked, this, &CommandLineFrontEnd::OnTerminate);
	connect(_ui.Clear, &QPushButton::clicked, this, &CommandLineFrontEnd::OnClear);

	connect(_process, &QProcess::readyReadStandardOutput, this, &CommandLineFrontEnd::OnReadyReadOutput);
	connect(_process, &QProcess::readyReadStandardError, this, &CommandLineFrontEnd::OnReadyReadError);
	connect(_process, &QProcess::errorOccurred, this, &CommandLineFrontEnd::OnErrorOccurred);
	connect(_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &CommandLineFrontEnd::OnCompilationFinished);
}

CommandLineFrontEnd::~CommandLineFrontEnd() = default;

void CommandLineFrontEnd::closeEvent(QCloseEvent* event)
{
	//Don't allow closing while a process is running
	if (_currentFile != -1)
	{
		return;
	}

	QDialog::closeEvent(event);
}

QString CommandLineFrontEnd::GetProgram() const
{
	return _ui.ProgramPath->text();
}

void CommandLineFrontEnd::SetProgram(const QString& fileName, const QString& filter)
{
	_ui.ProgramPath->setText(fileName);
	_programFilter = filter;
}

void CommandLineFrontEnd::SetInputFileFilter(const QString& filter)
{
	_inputFileFilter = filter;
}

void CommandLineFrontEnd::SetSettingsWidget(QWidget* widget)
{
	if (_settingsWidget == widget)
	{
		return;
	}

	delete _settingsWidget;
	_settingsWidget = nullptr;

	if (!widget)
	{
		return;
	}

	_settingsWidget = widget;
	_settingsWidget->setParent(_ui.ProgramSettings);

	delete _ui.ProgramSettings->layout();

	const auto layout = new QVBoxLayout(_ui.ProgramSettings);
	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(_settingsWidget);

	_ui.ProgramSettings->setLayout(layout);
}

void CommandLineFrontEnd::UpdateCompileSettings()
{
	_ui.CommandLineSettingsWidget->setEnabled(!_ui.ProgramPath->text().isEmpty());
	UpdateCompleteCommandLine();
}

void CommandLineFrontEnd::OnBrowseCompiler()
{
	const QString fileName{QFileDialog::getOpenFileName(
		this, "Select Program", _ui.ProgramPath->text(), _programFilter)};

	if (!fileName.isEmpty())
	{
		_ui.ProgramPath->setText(fileName);
	}
}

void CommandLineFrontEnd::OnAddFile()
{
	const QString fileName{QFileDialog::getOpenFileName(this, {}, {}, _inputFileFilter)};

	if (!fileName.isEmpty())
	{
		auto item = new QListWidgetItem(fileName);

		item->setFlags(item->flags() | Qt::ItemFlag::ItemIsEditable);

		_ui.Files->addItem(item);

		_ui.Compile->setEnabled(true);
	}
}

void CommandLineFrontEnd::OnRemoveFile()
{
	delete _ui.Files->takeItem(_ui.Files->currentRow());

	_ui.Compile->setEnabled(_ui.Files->count() > 0);
}

void CommandLineFrontEnd::OnCurrentFileChanged(QListWidgetItem* current)
{
	//Block signals from the list so we don't get another currentItemChanged signal during removal due to a race condition caused by a focus event
	//This signal would otherwise tell us that the item being removed is becoming current again
	const QSignalBlocker blocker{_ui.Files};
	_ui.RemoveFile->setEnabled(current != nullptr);
}

QStringList CommandLineFrontEnd::GetArguments()
{
	QStringList arguments;

	GetArgumentsCore(arguments);

	//Parse the additional arguments using basic quoted string-aware, whitespace delimited parsing
	{
		const QString additionalArguments{_ui.AdditionalCommandLineArguments->toPlainText()};

		bool inQuotes{false};

		QString argument;

		for (int i = 0; i < additionalArguments.size(); ++i)
		{
			const QChar c{additionalArguments[i]};

			if (c == '\"')
			{
				inQuotes = !inQuotes;
			}
			else if (!inQuotes && c.isSpace())
			{
				if (!argument.isEmpty())
				{
					arguments.append(argument);
				}

				argument.clear();
			}
			else
			{
				argument += c;
			}
		}

		if (!argument.isEmpty())
		{
			arguments.append(argument);
		}
	}

	return arguments;
}

void CommandLineFrontEnd::UpdateCompleteCommandLine()
{
	auto arguments{GetArguments()};

	for (auto& argument : arguments)
	{
		//Add quotes around any arguments that have whitespace
		if (argument.contains(QRegularExpression{".*\\s+.*"}))
		{
			argument = QString{"\"%1\""}.arg(argument);
		}
	}

	_ui.CompleteCommandLine->setPlainText(QString{"\"%1\" %2"}.arg(_ui.ProgramPath->text()).arg(arguments.join(' ')));
}

void CommandLineFrontEnd::StartNextCompilation()
{
	_outputIsError = false;

	QString fileName{_ui.Files->item(_currentFile)->text()};

	const QFileInfo fileInfo{fileName};

	QString workingDirectory;

	//Default the working directory to the qc file directory
	if (_ui.OverrideWorkingDirectory->isChecked())
	{
		workingDirectory = _ui.WorkingDirectory->text();
	}
	else
	{
		workingDirectory = fileInfo.absolutePath();
	}

	_process->setWorkingDirectory(workingDirectory);

	auto arguments{GetArguments()};

	arguments.append(fileName);

	AppendRegularText(QString{"Command line parameters: %1 \"%2\"<br/>"}.arg(_ui.CompleteCommandLine->toPlainText()).arg(fileName));

	_process->start(_ui.ProgramPath->text(), arguments, QIODevice::ReadOnly);
}

void CommandLineFrontEnd::OnBrowseWorkingDirectory()
{
	const QString path{QFileDialog::getExistingDirectory(this, {}, _ui.WorkingDirectory->text())};

	if (!path.isEmpty())
	{
		_ui.WorkingDirectory->setText(path);
	}
}

void CommandLineFrontEnd::OnCompile()
{
	//Freeze the settings during compilation
	_ui.CommandLinePathWidget->setEnabled(false);
	_ui.CommandLineSettingsWidget->setEnabled(false);

	_ui.Terminate->setEnabled(true);

	_currentFile = 0;

	StartNextCompilation();
}

void CommandLineFrontEnd::OnTerminate()
{
	_process->kill();
}

void CommandLineFrontEnd::OnClear()
{
	_ui.Output->clear();
}

void CommandLineFrontEnd::ScrollOutputToBottom()
{
	_ui.Output->verticalScrollBar()->setValue(_ui.Output->verticalScrollBar()->maximum());
}

void CommandLineFrontEnd::AppendRegularText(const QString& text)
{
	_ui.Output->append(text);
	ScrollOutputToBottom();
}

void CommandLineFrontEnd::AppendErrorText(QString text)
{
	//Convert newlines to HTML
	text.replace("\n", "<br/>");

	_ui.Output->insertHtml(QString{"<font color=\"red\">%1</font>"}.arg(text));
	ScrollOutputToBottom();
}

void CommandLineFrontEnd::Reset()
{
	_currentFile = -1;
	_ui.CommandLinePathWidget->setEnabled(true);
	UpdateCompileSettings();
	_ui.Terminate->setEnabled(false);
}

void CommandLineFrontEnd::OnReadyReadOutput()
{
	const QString output{_process->readAllStandardOutput()};

	//Studiomdl's Error function doesn't use stderr so we have to detect error output manually
	const int errorIndex = output.indexOf("************ ERROR ************");

	if (errorIndex != -1)
	{
		_outputIsError = true;
		AppendRegularText(output.left(errorIndex));
		AppendErrorText(output.right(output.size() - errorIndex));
		return;
	}

	if (_outputIsError)
	{
		AppendErrorText(output);
	}
	else
	{
		AppendRegularText(output);
	}
}

void CommandLineFrontEnd::OnReadyReadError()
{
	AppendErrorText(_process->readAllStandardError());
}

void CommandLineFrontEnd::OnErrorOccurred(QProcess::ProcessError error)
{
	Reset();

	switch (error)
	{
	case QProcess::ProcessError::FailedToStart:
	{
		AppendErrorText("<br/>Process failed to start<br/>");
		break;
	}
	case QProcess::ProcessError::Crashed:
	{
		AppendErrorText("<br/>Process crashed<br/>");
		break;
	}

	case QProcess::ProcessError::Timedout:
	{
		//Technically not a fatal error but since we don't use waitFor* methods it will be treated as such
		AppendErrorText("<br/>Timed out<br/>");
		break;
	}

	case QProcess::ProcessError::ReadError:
	{
		AppendErrorText("<br/>Read error<br/>");
		break;
	}

	case QProcess::ProcessError::WriteError:
	{
		AppendErrorText("<br/>Write error<br/>");
		break;
	}

	case QProcess::ProcessError::UnknownError:
	{
		AppendErrorText("<br/>Unknown error<br/>");
		break;
	}
	}

	//Kill the process if it's still running
	if (_process->state() != QProcess::ProcessState::NotRunning)
	{
		_process->kill();
	}
}

void CommandLineFrontEnd::OnCompilationFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	//If this is true then an error occurred and we're wrapping up execution of the process without continuing
	if (_currentFile == -1)
	{
		return;
	}

	switch (exitStatus)
	{
	case QProcess::ExitStatus::NormalExit:
	{
		AppendRegularText(QString{"<br/>The program exited normally with exit code %1<br/>"}.arg(exitCode));
		break;
	}

	case QProcess::ExitStatus::CrashExit:
	{
		AppendRegularText("<br/>The program crashed<br/>");
		break;
	}
	}

	++_currentFile;

	if (_currentFile >= _ui.Files->count())
	{
		Reset();
		return;
	}

	StartNextCompilation();
}
}
