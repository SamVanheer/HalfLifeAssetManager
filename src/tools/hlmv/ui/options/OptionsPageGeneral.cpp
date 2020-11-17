#include "ui/options/OptionsPageGeneral.hpp"

namespace ui::options
{
OptionsPageGeneral::OptionsPageGeneral(QWidget* parent)
	: QWidget(parent)
{
	_ui.setupUi(this);
}

OptionsPageGeneral::~OptionsPageGeneral() = default;

void OptionsPageGeneral::OnSaveChanges(QSettings& settings)
{
	//TODO
}
}
