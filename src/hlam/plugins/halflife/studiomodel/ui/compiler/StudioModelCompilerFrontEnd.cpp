#include <QSignalBlocker>
#include <QString>

#include "plugins/halflife/studiomodel/ui/compiler/StudioModelCompilerFrontEnd.hpp"
#include "settings/ApplicationSettings.hpp"
#include "settings/ExternalProgramSettings.hpp"

#include "application/AssetManager.hpp"
//TODO: move exe filter constant somewhere else
#include "ui/options/OptionsPageExternalPrograms.hpp"

namespace studiomodel
{
StudioModelCompilerFrontEnd::StudioModelCompilerFrontEnd(AssetManager* application)
	: CommandLineFrontEnd(application)
{
	_settingsWidget = new QWidget(this);
	_settingsUi.setupUi(_settingsWidget);

	_settingsUi.RemoveTextureReplacement->setEnabled(false);

	connect(_settingsUi.TagReversedTriangles, &QCheckBox::stateChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_settingsUi.TagReversedNormals, &QCheckBox::stateChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_settingsUi.DumpHitboxes, &QCheckBox::stateChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_settingsUi.IgnoreWarnings, &QCheckBox::stateChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);

	connect(_settingsUi.AddVertexBlendAngle, &QGroupBox::toggled, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_settingsUi.VertexBlendAngle, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_settingsUi.AddMaximumSequenceGroupSize, &QGroupBox::toggled, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);
	connect(_settingsUi.MaximumSequenceGroupSize, qOverload<int>(&QSpinBox::valueChanged), this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);

	connect(_settingsUi.AddTextureReplacement, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnAddTextureReplacement);
	connect(_settingsUi.RemoveTextureReplacement, &QPushButton::clicked, this, &StudioModelCompilerFrontEnd::OnRemoveTextureReplacement);
	connect(_settingsUi.TextureReplacements, &QTableWidget::currentItemChanged, this, &StudioModelCompilerFrontEnd::OnCurrentTextureReplacementChanged);
	connect(_settingsUi.TextureReplacements, &QTableWidget::cellChanged, this, &StudioModelCompilerFrontEnd::UpdateCompleteCommandLine);

	SetProgram(_application->GetApplicationSettings()->GetExternalPrograms()->GetProgram(StudiomdlCompilerFileNameKey),
		ExternalProgramsExeFilter);
	SetInputFileFilter("QC Files (*.qc);;All Files (*.*)");
	SetSettingsWidget(_settingsWidget);
}

StudioModelCompilerFrontEnd::~StudioModelCompilerFrontEnd()
{
	//Sync any changes made to settings
	_application->GetApplicationSettings()->GetExternalPrograms()->SetProgram(
		StudiomdlCompilerFileNameKey, GetProgram());

	_application->GetApplicationSettings()->SaveSettings();
}

void StudioModelCompilerFrontEnd::GetArgumentsCore(QStringList& arguments)
{
	if (_settingsUi.TagReversedTriangles->isChecked())
	{
		arguments.append("-r");
	}

	if (_settingsUi.TagReversedNormals->isChecked())
	{
		arguments.append("-n");
	}

	if (_settingsUi.DumpHitboxes->isChecked())
	{
		arguments.append("-h");
	}

	if (_settingsUi.IgnoreWarnings->isChecked())
	{
		arguments.append("-i");
	}

	if (_settingsUi.AddVertexBlendAngle->isChecked())
	{
		arguments.append("-a");
		arguments.append(QString::number(_settingsUi.VertexBlendAngle->value(), 'f'));
	}

	if (_settingsUi.AddMaximumSequenceGroupSize->isChecked())
	{
		arguments.append("-g");
		arguments.append(QString::number(_settingsUi.MaximumSequenceGroupSize->value()));
	}

	for (int i = 0; i < _settingsUi.TextureReplacements->rowCount(); ++i)
	{
		//The first string is the replacement; the second is the original to match.
		auto original = _settingsUi.TextureReplacements->item(i, 0);
		auto replacement = _settingsUi.TextureReplacements->item(i, 1);

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
}

void StudioModelCompilerFrontEnd::OnAddTextureReplacement()
{
	const int row = _settingsUi.TextureReplacements->rowCount();

	_settingsUi.TextureReplacements->insertRow(row);

	//TODO: studiomdl stores texture names in arrays of size 16 bytes, and does not perform bounds checking during copying
	//Try to protect users from data corruption

	auto original = new QTableWidgetItem("Original.bmp");

	original->setFlags(original->flags() | Qt::ItemFlag::ItemIsEditable);

	_settingsUi.TextureReplacements->setItem(row, 0, original);

	auto replacement = new QTableWidgetItem("Replacement.bmp");

	replacement->setFlags(replacement->flags() | Qt::ItemFlag::ItemIsEditable);

	_settingsUi.TextureReplacements->setItem(row, 1, replacement);
}

void StudioModelCompilerFrontEnd::OnRemoveTextureReplacement()
{
	_settingsUi.TextureReplacements->removeRow(_settingsUi.TextureReplacements->currentRow());
	UpdateCompleteCommandLine();
}

void StudioModelCompilerFrontEnd::OnCurrentTextureReplacementChanged(QTableWidgetItem* current)
{
	const QSignalBlocker blocker{_settingsUi.TextureReplacements};
	_settingsUi.RemoveTextureReplacement->setEnabled(current != nullptr);
}
}
