#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QString>

#include "ui/EditorContext.hpp"
#include "ui/options/OptionsPageStudioModel.hpp"
#include "ui/settings/StudioModelSettings.hpp"
#include "ui/assets/studiomodel/compiler/StudioModelCompilerFrontEnd.hpp"

namespace ui::assets::studiomodel
{
StudioModelCompilerFrontEnd::StudioModelCompilerFrontEnd(EditorContext* editorContext, settings::StudioModelSettings* studioModelSettings, QWidget* parent)
	: QDialog(parent)
	, _editorContext(editorContext)
	, _studioModelSettings(studioModelSettings)
{
	_ui.setupUi(this);

	//Set the dialog size to a fraction of the current screen
	const QRect screenSize = QApplication::desktop()->screenGeometry(this);

	this->resize(static_cast<int>(std::ceil(screenSize.width() * 0.9)), static_cast<int>(std::ceil(screenSize.height() * 0.9)));

	_ui.RemoveQCFile->setEnabled(false);
	_ui.RemoveTextureReplacement->setEnabled(false);
	_ui.Compile->setEnabled(false);
	_ui.Terminate->setEnabled(false);

	connect(_ui.Compiler, &QLineEdit::textChanged, this, &StudioModelCompilerFrontEnd::UpdateCompileSettings);
	connect(_ui.BrowseCompiler, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnBrowseCompiler);

	connect(_ui.AddQCFile, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnAddQCFile);
	connect(_ui.RemoveQCFile, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnRemoveQCFile);
	connect(_ui.QCFiles, &QListWidget::currentItemChanged, this, &StudioModelCompilerFrontEnd::OnCurrentQCFileChanged);

	connect(_ui.BrowseWorkingDirectory, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnBrowseWorkingDirectory);

	connect(_ui.TagReversedTriangles, &QCheckBox::stateChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_ui.TagReversedNormals, &QCheckBox::stateChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_ui.DumpHitboxes, &QCheckBox::stateChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_ui.IgnoreWarnings, &QCheckBox::stateChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);

	connect(_ui.AddVertexBlendAngle, &QGroupBox::toggled, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_ui.VertexBlendAngle, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_ui.AddMaximumSequenceGroupSize, &QGroupBox::toggled, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_ui.MaximumSequenceGroupSize, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);

	connect(_ui.AddTextureReplacement, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnAddTextureReplacement);
	connect(_ui.RemoveTextureReplacement, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnRemoveTextureReplacement);
	connect(_ui.TextureReplacements, &QTableWidget::currentItemChanged, this, &StudioModelCompilerFrontEnd::OnCurrentTextureReplacementChanged);
	connect(_ui.TextureReplacements, &QTableWidget::cellChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);

	//TODO: maybe have special change handling for command line arguments to avoid having to reparse it every time
	connect(_ui.AdditionalCommandLineArguments, &QPlainTextEdit::textChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);

	connect(_ui.Compile, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnCompile);
	connect(_ui.Terminate, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnTerminate);
	connect(_ui.Clear, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnClear);

	connect(_process, &QProcess::readyReadStandardOutput, this, &StudioModelCompilerFrontEnd::OnReadyReadOutput);
	connect(_process, &QProcess::readyReadStandardError, this, &StudioModelCompilerFrontEnd::OnReadyReadError);
	connect(_process, &QProcess::errorOccurred, this, &StudioModelCompilerFrontEnd::OnErrorOccurred);
	connect(_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &StudioModelCompilerFrontEnd::OnCompilationFinished);

	_ui.Compiler->setText(_studioModelSettings->GetStudiomdlCompilerFileName());
}

StudioModelCompilerFrontEnd::~StudioModelCompilerFrontEnd()
{
	//Sync any changes made to settings
	_studioModelSettings->SetStudiomdlCompilerFileName(_ui.Compiler->text());

	_studioModelSettings->SaveSettings(*_editorContext->GetSettings());
}

void StudioModelCompilerFrontEnd::closeEvent(QCloseEvent* event)
{
	//Don't allow closing while a process is running
	if (_currentQCFile != -1)
	{
		return;
	}

	QDialog::closeEvent(event);
}

void StudioModelCompilerFrontEnd::UpdateCompileSettings()
{
	_ui.CompileSettingsWidget->setEnabled(!_ui.Compiler->text().isEmpty());
	UpdateCompleteCommandLine();
}

void StudioModelCompilerFrontEnd::OnBrowseCompiler()
{
	const QString fileName{QFileDialog::getOpenFileName(
		this, "Select Studiomdl Compiler", _ui.Compiler->text(), options::StudioModelExeFilter)};

	if (!fileName.isEmpty())
	{
		_ui.Compiler->setText(fileName);
	}
}

void StudioModelCompilerFrontEnd::OnAddQCFile()
{
	const QString fileName{QFileDialog::getOpenFileName(this, {}, {}, "QC Files (*.qc);;All Files (*.*)")};

	if (!fileName.isEmpty())
	{
		auto item = new QListWidgetItem(fileName);

		item->setFlags(item->flags() | Qt::ItemFlag::ItemIsEditable);

		_ui.QCFiles->addItem(item);

		_ui.Compile->setEnabled(true);
	}
}

void StudioModelCompilerFrontEnd::OnRemoveQCFile()
{
	delete _ui.QCFiles->takeItem(_ui.QCFiles->currentRow());

	_ui.Compile->setEnabled(_ui.QCFiles->count() > 0);
}

void StudioModelCompilerFrontEnd::OnCurrentQCFileChanged(QListWidgetItem* current)
{
	//Block signals from the list so we don't get another currentItemChanged signal during removal due to a race condition caused by a focus event
	//This signal would otherwise tell us that the item being removed is becoming current again
	const QSignalBlocker blocker{_ui.QCFiles};
	_ui.RemoveQCFile->setEnabled(current != nullptr);
}

void StudioModelCompilerFrontEnd::OnBrowseWorkingDirectory()
{
	const QString path{QFileDialog::getExistingDirectory(this, {}, _ui.WorkingDirectory->text())};

	if (!path.isEmpty())
	{
		_ui.WorkingDirectory->setText(path);
	}
}

void StudioModelCompilerFrontEnd::OnAddTextureReplacement()
{
	const int row = _ui.TextureReplacements->rowCount();

	_ui.TextureReplacements->insertRow(row);

	//TODO: studiomdl stores texture names in arrays of size 16 bytes, and does not perform bounds checking during copying
	//Try to protect users from data corruption

	auto original = new QTableWidgetItem("Original.bmp");

	original->setFlags(original->flags() | Qt::ItemFlag::ItemIsEditable);

	_ui.TextureReplacements->setItem(row, 0, original);

	auto replacement = new QTableWidgetItem("Replacement.bmp");

	replacement->setFlags(replacement->flags() | Qt::ItemFlag::ItemIsEditable);

	_ui.TextureReplacements->setItem(row, 1, replacement);
}

void StudioModelCompilerFrontEnd::OnRemoveTextureReplacement()
{
	_ui.TextureReplacements->removeRow(_ui.TextureReplacements->currentRow());
	UpdateCompleteCommandLine();
}

void StudioModelCompilerFrontEnd::OnCurrentTextureReplacementChanged(QTableWidgetItem* current)
{
	const QSignalBlocker blocker{_ui.TextureReplacements};
	_ui.RemoveTextureReplacement->setEnabled(current != nullptr);
}

QStringList StudioModelCompilerFrontEnd::GetArguments()
{
	QStringList arguments;

	if (_ui.TagReversedTriangles->isChecked())
	{
		arguments.append("-r");
	}

	if (_ui.TagReversedNormals->isChecked())
	{
		arguments.append("-n");
	}

	if (_ui.DumpHitboxes->isChecked())
	{
		arguments.append("-h");
	}

	if (_ui.IgnoreWarnings->isChecked())
	{
		arguments.append("-i");
	}

	if (_ui.AddVertexBlendAngle->isChecked())
	{
		arguments.append("-a");
		arguments.append(QString::number(_ui.VertexBlendAngle->value()));
	}

	if (_ui.AddMaximumSequenceGroupSize->isChecked())
	{
		arguments.append("-g");
		arguments.append(QString::number(_ui.MaximumSequenceGroupSize->value()));
	}

	for (int i = 0; i < _ui.TextureReplacements->rowCount(); ++i)
	{
		//The first string is the replacement; the second is the original to match.
		auto original = _ui.TextureReplacements->item(i, 0);
		auto replacement = _ui.TextureReplacements->item(i, 1);

		//We can get here when a new row is added, but before items are inserted. Make sure both are valid.
		if (original && replacement)
		{
			arguments.append("-t");
			arguments.append(replacement->text());

			const QString sourceTexture = original->text();

			if (!sourceTexture.isEmpty())
			{
				arguments.append(sourceTexture);
			}
		}
	}

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

void StudioModelCompilerFrontEnd::UpdateCompleteCommandLine()
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

	_ui.CompleteCommandLine->setPlainText(QString{"\"%1\" %2"}.arg(_ui.Compiler->text()).arg(arguments.join(' ')));
}

void StudioModelCompilerFrontEnd::StartNextCompilation()
{
	_outputIsError = false;

	QString fileName{_ui.QCFiles->item(_currentQCFile)->text()};

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

	AppendRegularText(QString{"Command line parameters: %1<br/>"}.arg(_ui.CompleteCommandLine->toPlainText()));

	_process->start(_ui.Compiler->text(), arguments, QIODevice::ReadOnly);
}

void StudioModelCompilerFrontEnd::OnCompile()
{
	//Freeze the settings during compilation
	_ui.CompilerPathWidget->setEnabled(false);
	_ui.CompileSettingsWidget->setEnabled(false);

	_ui.Terminate->setEnabled(true);

	_currentQCFile = 0;

	StartNextCompilation();
}

void StudioModelCompilerFrontEnd::OnTerminate()
{
	_process->kill();
}

void StudioModelCompilerFrontEnd::OnClear()
{
	_ui.Output->clear();
}

void StudioModelCompilerFrontEnd::ScrollOutputToBottom()
{
	_ui.Output->verticalScrollBar()->setValue(_ui.Output->verticalScrollBar()->maximum());
}

void StudioModelCompilerFrontEnd::AppendRegularText(const QString& text)
{
	_ui.Output->append(text);
	ScrollOutputToBottom();
}

void StudioModelCompilerFrontEnd::AppendErrorText(QString text)
{
	//Convert newlines to HTML
	text.replace("\n", "<br/>");

	_ui.Output->insertHtml(QString{"<font color=\"red\">%1</font>"}.arg(text));
	ScrollOutputToBottom();
}

void StudioModelCompilerFrontEnd::Reset()
{
	_currentQCFile = -1;
	_ui.CompilerPathWidget->setEnabled(true);
	UpdateCompileSettings();
	_ui.Terminate->setEnabled(false);
}

void StudioModelCompilerFrontEnd::OnReadyReadOutput()
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

void StudioModelCompilerFrontEnd::OnReadyReadError()
{
	AppendErrorText(_process->readAllStandardError());
}

void StudioModelCompilerFrontEnd::OnErrorOccurred(QProcess::ProcessError error)
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

void StudioModelCompilerFrontEnd::OnCompilationFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	//If this is true then an error occurred and we're wrapping up execution of the process without continuing
	if (_currentQCFile == -1)
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

	++_currentQCFile;

	if (_currentQCFile >= _ui.QCFiles->count())
	{
		Reset();
		return;
	}

	StartNextCompilation();
}
}
