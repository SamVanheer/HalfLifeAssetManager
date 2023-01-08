#pragma once

#include <QDialog>
#include <QProcess>
#include <QStringList>

#include "ui_CommandLineFrontEnd.h"

class QListWidgetItem;
class QWidget;

class AssetManager;

namespace studiomodel
{
class CommandLineFrontEnd : public QDialog
{
public:
	CommandLineFrontEnd(AssetManager* application);
	~CommandLineFrontEnd();

protected:
	void closeEvent(QCloseEvent* event) override;

	QString GetProgram() const;

	void SetProgram(const QString& fileName, const QString& filter);

	void SetInputFileFilter(const QString& filter);

	void SetSettingsWidget(QWidget* widget);

	virtual void GetArgumentsCore(QStringList& arguments) {}

private:
	QStringList GetArguments();

	void StartNextCompilation();

	void ScrollOutputToBottom();

	void AppendRegularText(const QString& text);
	void AppendErrorText(QString text);

	void Reset();

protected slots:
	void UpdateCompleteCommandLine();

private slots:
	void UpdateCompileSettings();

	void OnBrowseCompiler();

	void OnAddFile();
	void OnRemoveFile();

	void OnCurrentFileChanged(QListWidgetItem* current);

	void OnBrowseWorkingDirectory();

	void OnCompile();
	void OnTerminate();
	void OnClear();

	void OnReadyReadOutput();
	void OnReadyReadError();

	void OnErrorOccurred(QProcess::ProcessError error);

	void OnCompilationFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
	AssetManager* const _application;

private:
	Ui_CommandLineFrontEnd _ui;

	QProcess* const _process = new QProcess(this);

	QString _programFilter;
	QString _inputFileFilter;

	QWidget* _settingsWidget{};

	int _currentFile{-1};

	bool _outputIsError{false};
};
}
