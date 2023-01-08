#pragma once

#include <QString>
#include <QStringList>
#include <QWidget>

#include "plugins/halflife/studiomodel/ui/compiler/CommandLineFrontEnd.hpp"

namespace studiomodel
{
const inline QString StudiomdlDecompilerFileNameKey{QStringLiteral("StudioMdlDecompilerFileName")};

class StudioModelDecompilerFrontEnd final : public CommandLineFrontEnd
{
public:
	StudioModelDecompilerFrontEnd(AssetManager* application);
	~StudioModelDecompilerFrontEnd();
};
}
