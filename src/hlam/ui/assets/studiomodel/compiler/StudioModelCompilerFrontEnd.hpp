#pragma once

#include <QStringList>
#include <QWidget>

#include "ui_StudioModelCompilerFrontEnd.h"

#include "ui/assets/studiomodel/compiler/CommandLineFrontEnd.hpp"

namespace studiomodel
{
class StudioModelCompilerFrontEnd final : public CommandLineFrontEnd
{
public:
	StudioModelCompilerFrontEnd(EditorContext* editorContext);
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
