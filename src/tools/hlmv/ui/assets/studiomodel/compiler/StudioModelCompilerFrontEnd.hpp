#pragma once

#include <QDialog>
#include <QProcess>
#include <QStringList>

#include "ui_StudioModelCompilerFrontEnd.h"

class QListWidgetItem;

namespace ui
{
class EditorContext;

namespace settings
{
class StudioModelSettings;
}

namespace assets::studiomodel
{
class StudioModelCompilerFrontEnd final : public QDialog
{
public:
	StudioModelCompilerFrontEnd(EditorContext* editorContext, settings::StudioModelSettings* studioModelSettings, QWidget* parent = nullptr);
	~StudioModelCompilerFrontEnd();

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	QStringList GetArguments();

	void StartNextCompilation();

	void ScrollOutputToBottom();

	void AppendRegularText(const QString& text);
	void AppendErrorText(QString text);

	void Reset();

private slots:
	void UpdateCompileSettings();
	void OnBrowseCompiler();

	void OnAddQCFile();
	void OnRemoveQCFile();

	void OnCurrentQCFileChanged(QListWidgetItem* current);

	void OnBrowseWorkingDirectory();

	void OnAddTextureReplacement();
	void OnRemoveTextureReplacement();

	void OnCurrentTextureReplacementChanged(QTableWidgetItem* current);

	void UpdateCompleteCommandLine();

	void OnCompile();
	void OnTerminate();
	void OnClear();

	void OnReadyReadOutput();
	void OnReadyReadError();

	void OnErrorOccurred(QProcess::ProcessError error);

	void OnCompilationFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
	Ui_StudioModelCompilerFrontEnd _ui;

	EditorContext* const _editorContext;

	settings::StudioModelSettings* const _studioModelSettings;

	QProcess* const _process = new QProcess(this);

	int _currentQCFile{-1};

	bool _outputIsError{false};
};
}
}
