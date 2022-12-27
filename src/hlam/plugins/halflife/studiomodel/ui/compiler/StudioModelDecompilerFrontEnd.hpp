#pragma once

#include <QStringList>
#include <QWidget>

#include "plugins/halflife/studiomodel/ui/compiler/CommandLineFrontEnd.hpp"

namespace studiomodel
{
class StudioModelDecompilerFrontEnd final : public CommandLineFrontEnd
{
public:
	StudioModelDecompilerFrontEnd(EditorContext* editorContext);
	~StudioModelDecompilerFrontEnd();
};
}
