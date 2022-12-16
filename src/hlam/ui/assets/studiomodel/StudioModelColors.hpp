#pragma once

#include <QString>

#include <glm/vec3.hpp>

#include "ui/settings/ColorSettings.hpp"

namespace ui::assets::studiomodel
{
struct ColorInfo final
{
	const QString Name;
	const glm::vec3 DefaultColor;
};

const inline ColorInfo GroundColor{QStringLiteral("Ground"), settings::RGB888ToVector(216, 216, 175)};
const inline ColorInfo BackgroundColor{QStringLiteral("Background"), settings::RGB888ToVector(63, 127, 127)};
const inline ColorInfo CrosshairColor{QStringLiteral("Crosshair"), settings::RGB888ToVector(255, 0, 0)};
const inline ColorInfo LightColor{QStringLiteral("Light"), settings::RGB888ToVector(255, 255, 255)};
const inline ColorInfo WireframeColor{QStringLiteral("Wireframe"), settings::RGB888ToVector(255, 0, 0)};
}
