#pragma once

#include <QStringList>
#include <QWidget>

#include "ui/assets/studiomodel/compiler/CommandLineFrontEnd.hpp"

namespace ui
{
namespace settings
{
class StudioModelSettings;
}

namespace assets::studiomodel
{
class StudioModelDecompilerFrontEnd final : public CommandLineFrontEnd
{
public:
	StudioModelDecompilerFrontEnd(EditorContext* editorContext, settings::StudioModelSettings* studioModelSettings);
	~StudioModelDecompilerFrontEnd();

private:
	settings::StudioModelSettings* const _studioModelSettings;
};
}
}
