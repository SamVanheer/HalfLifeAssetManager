#pragma once

#include <QString>
#include <QStringList>
#include <QWidget>

#include "ui_StudioModelCompilerFrontEnd.h"

#include "plugins/halflife/studiomodel/ui/compiler/CommandLineFrontEnd.hpp"

namespace studiomodel
{
const inline QString StudiomdlCompilerFileNameKey{QStringLiteral("StudioMdlCompilerFileName")};

class StudioModelCompilerFrontEnd final : public CommandLineFrontEnd
{
public:
	StudioModelCompilerFrontEnd(AssetManager* application);
	~StudioModelCompilerFrontEnd();

protected:
	void GetArgumentsCore(QStringList& arguments) override;

private slots:
	void OnAddTextureReplacement();
	void OnRemoveTextureReplacement();

	void OnCurrentTextureReplacementChanged(QTableWidgetItem* current);

private:
	QWidget* _settingsWidget;
	Ui_StudioModelCompilerFrontEnd _settingsUi;
};
}
