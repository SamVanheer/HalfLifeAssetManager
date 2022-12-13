#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <tuple>

#include <QColor>
#include <QImage>
#include <QRgb>
#include <QString>

#include "formats/studiomodel/EditableStudioModel.hpp"
#include "graphics/Palette.hpp"

namespace ui
{
QString FormatTextureName(const studiomdl::Texture& texture);

/**
*	@brief Converts an image to an indexed 8 bit image compatible with GoldSource
*	@return If conversion succeeded, the converted texture and whether the image was converted from another format to index 8 bit
*/
std::optional<std::tuple<studiomdl::TextureData, bool>> ConvertImageToTexture(QImage image);

QImage ConvertTextureToRGBImage(
	const studiomdl::TextureData& texture, const std::byte* textureData, const graphics::RGBPalette& texturePalette, std::vector<QRgb>& dataBuffer);

QImage ConvertTextureToIndexed8Image(const studiomdl::TextureData& texture);

QImage CreateUVMapImage(
	const studiomdl::EditableStudioModel& model, int textureIndex, int meshIndex, bool antiAliasLines, float textureScale, qreal lineWidth);

void DrawUVImage(const QColor& backgroundColor, bool showUVMap, bool overlayOnTexture, const QImage& texture, const QImage& uvMap, QImage& target);
}
