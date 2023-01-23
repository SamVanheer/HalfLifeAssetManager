#pragma once

#include <memory>
#include <vector>

#include <QDialog>

class AssetManager;
class Ui_OpenInExternalProgramDialog;

struct ExternalProgramCommand
{
	QString FileName;
	QString ExternalProgramKey;
};

class OpenInExternalProgramDialog final : public QDialog
{
public:
	explicit OpenInExternalProgramDialog(AssetManager* application, QWidget* parent,
		const std::vector<ExternalProgramCommand>& commands);
	~OpenInExternalProgramDialog();

private:
	void OpenRow(int row);

private slots:
	void OpenSelectedFile();

private:
	std::unique_ptr<Ui_OpenInExternalProgramDialog> _ui;
	AssetManager* const _application;
};
