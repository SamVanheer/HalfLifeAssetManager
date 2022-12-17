#pragma once

#include <QString>

#include <glm/vec3.hpp>

#include "ui/settings/ColorSettings.hpp"

namespace studiomodel
{
struct ColorInfo final
{
	const QString Name;
	const glm::vec3 DefaultColor;
};

const inline ColorInfo GroundColor{QStringLiteral("Ground"), RGB888ToVector(216, 216, 175)};
const inline ColorInfo BackgroundColor{QStringLiteral("Background"), RGB888ToVector(63, 127, 127)};
const inline ColorInfo CrosshairColor{QStringLiteral("Crosshair"), RGB888ToVector(255, 0, 0)};
const inline ColorInfo LightColor{QStringLiteral("Light"), RGB888ToVector(255, 255, 255)};
const inline ColorInfo WireframeColor{QStringLiteral("Wireframe"), RGB888ToVector(255, 0, 0)};
}
