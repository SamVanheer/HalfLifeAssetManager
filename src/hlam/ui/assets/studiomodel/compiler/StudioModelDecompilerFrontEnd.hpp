#pragma once

#include <QStringList>
#include <QWidget>

#include "ui/assets/studiomodel/compiler/CommandLineFrontEnd.hpp"

namespace studiomodel
{
class StudioModelDecompilerFrontEnd final : public CommandLineFrontEnd
{
public:
	StudioModelDecompilerFrontEnd(EditorContext* editorContext);
	~StudioModelDecompilerFrontEnd();
};
}
