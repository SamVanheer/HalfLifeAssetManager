#pragma once

#include <QColor>
#include <QString>

#include <glm/vec3.hpp>

namespace ui::assets::studiomodel
{
struct ColorInfo final
{
	const QString Name;
	const QColor DefaultColor;
};

const inline ColorInfo GroundColor{QStringLiteral("Ground"), QColor::fromRgb(216, 216, 175)};
const inline ColorInfo BackgroundColor{QStringLiteral("Background"), QColor::fromRgb(63, 127, 127)};
const inline ColorInfo CrosshairColor{QStringLiteral("Crosshair"), QColor::fromRgb(255, 0, 0)};
const inline ColorInfo LightColor{QStringLiteral("Light"), QColor::fromRgb(255, 255, 255)};
const inline ColorInfo WireframeColor{QStringLiteral("Wireframe"), QColor::fromRgb(255, 0, 0)};

// TODO: maybe store the vec3 version in the settings object
inline glm::vec3 ColorToVector(const QColor& color)
{
	return {color.redF(), color.greenF(), color.blueF()};
}
}
