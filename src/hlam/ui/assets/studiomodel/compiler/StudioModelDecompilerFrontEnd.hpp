#pragma once

#include <QStringList>
#include <QWidget>

#include "ui/assets/studiomodel/compiler/CommandLineFrontEnd.hpp"

class StudioModelSettings;

namespace studiomodel
{
class StudioModelDecompilerFrontEnd final : public CommandLineFrontEnd
{
public:
	StudioModelDecompilerFrontEnd(EditorContext* editorContext, StudioModelSettings* studioModelSettings);
	~StudioModelDecompilerFrontEnd();

private:
	StudioModelSettings* const _studioModelSettings;
};
}
