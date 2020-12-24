#pragma once

#include <QStringList>
#include <QWidget>

#include "ui_StudioModelCompilerFrontEnd.h"

#include "ui/assets/studiomodel/compiler/CommandLineFrontEnd.hpp"

namespace ui
{
namespace settings
{
class StudioModelSettings;
}

namespace assets::studiomodel
{
class StudioModelCompilerFrontEnd final : public CommandLineFrontEnd
{
public:
	StudioModelCompilerFrontEnd(EditorContext* editorContext, settings::StudioModelSettings* studioModelSettings, QWidget* parent = nullptr);
	~StudioModelCompilerFrontEnd();

protected:
	void GetArgumentsCore(QStringList& arguments) override;

private slots:
	void OnAddTextureReplacement();
	void OnRemoveTextureReplacement();

	void OnCurrentTextureReplacementChanged(QTableWidgetItem* current);

private:
	settings::StudioModelSettings* const _studioModelSettings;

	QWidget* _settingsWidget;
	Ui_StudioModelCompilerFrontEnd _settingsUi;
};
}
}
