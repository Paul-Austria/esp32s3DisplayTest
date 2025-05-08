#include "ScaleTextureRenderer.h"
#include <algorithm>
#include "../../util/MemHandler.h"
#include "../../data/BlendMode/BlendFunctions.h"
#include "../../data/PixelFormat/PixelConverter.h"
#include "../RenderContext2D.h"
#include <float.h>
#include <math.h>
using namespace Tergos2D;

ScaleTextureRenderer::ScaleTextureRenderer(RenderContext2D &context) : RendererBase(context)
{
}

void ScaleTextureRenderer::DrawTexture(Texture &texture, int16_t x, int16_t y,
                                       float scaleX, float scaleY)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture || !texture.GetData() || scaleX <= 0 || scaleY <= 0)
        return;

    if (scaleX == 1 && scaleY == 1)
    {
        context.basicTextureRenderer.DrawTexture(texture, x, y);
        return;
    }
    // Get format information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);

    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Get source texture information
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch();

    // Calculate scaled dimensions
    uint16_t dstWidth = static_cast<uint16_t>(sourceWidth * scaleX);
    uint16_t dstHeight = static_cast<uint16_t>(sourceHeight * scaleY);

    // Set clipping boundaries based on SCALED size
    auto clippingArea = context.GetClippingArea();
    int16_t clipStartX = context.IsClippingEnabled() ? std::max(x, clippingArea.startX) : x;
    int16_t clipStartY = context.IsClippingEnabled() ? std::max(y, clippingArea.startY) : y;
    int16_t clipEndX = context.IsClippingEnabled()
                           ? std::min(static_cast<int>(x + dstWidth), static_cast<int>(clippingArea.endX))
                           : x + dstWidth;
    int16_t clipEndY = context.IsClippingEnabled()
                           ? std::min(static_cast<int>(y + dstHeight), static_cast<int>(clippingArea.endY))
                           : y + dstHeight;

    // Clamp to target texture bounds
    clipEndX = std::min(clipEndX, (int16_t)targetWidth);
    clipEndY = std::min(clipEndY, (int16_t)targetHeight);

    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
        return;

    // Prepare blending mode
    BlendContext bc = context.GetBlendContext();
    bc.mode = context.BlendModeToUse(sourceInfo);

    uint8_t dstBuffer[MAXBYTESPERPIXEL];

    for (int16_t dy = clipStartY; dy < clipEndY; dy++)
    {
        if (dy < 0)
            continue;
        for (int16_t dx = clipStartX; dx < clipEndX; dx++)
        {
            if (dx < 0)
                continue;
            // Calculate normalized texture coordinates with inverse scaling
            float tx = (dx - x) * (static_cast<float>(sourceWidth) / dstWidth);
            float ty = (dy - y) * (static_cast<float>(sourceHeight) / dstHeight);

            // Clamp coordinates to texture size
            tx = std::max(0.0f, std::min(tx, static_cast<float>(sourceWidth - 1)));
            ty = std::max(0.0f, std::min(ty, static_cast<float>(sourceHeight - 1)));

            // Sample texture
            switch (context.GetSamplingMethod())
            {
            case SamplingMethod::NEAREST:
            {
                // Nearest neighbor sampling
                uint16_t sx = static_cast<uint16_t>(tx + 0.5f);
                uint16_t sy = static_cast<uint16_t>(ty + 0.5f);
                const uint8_t *srcPixel = sourceData +
                                          sy * sourcePitch +
                                          sx * sourceInfo.bytesPerPixel;

                PixelConverter::Convert(
                    sourceFormat,
                    targetFormat,
                    srcPixel,
                    dstBuffer,
                    1);
                break;
            }

            case SamplingMethod::LINEAR:
            {
                // Bilinear interpolation
                int x0 = static_cast<int>(tx);
                int y0 = static_cast<int>(ty);
                int x1 = std::min(x0 + 1, static_cast<int>(sourceWidth - 1));
                int y1 = std::min(y0 + 1, static_cast<int>(sourceHeight - 1));

                float fx = tx - x0;
                float fy = ty - y0;

                // Get four neighboring pixels
                const uint8_t *pixels[4] = {
                    sourceData + y0 * sourcePitch + x0 * sourceInfo.bytesPerPixel, // (x0,y0)
                    sourceData + y0 * sourcePitch + x1 * sourceInfo.bytesPerPixel, // (x1,y0)
                    sourceData + y1 * sourcePitch + x0 * sourceInfo.bytesPerPixel, // (x0,y1)
                    sourceData + y1 * sourcePitch + x1 * sourceInfo.bytesPerPixel  // (x1,y1)
                };

                // Convert all four pixels to ARGB8888 color format
                Color colors[4];
                for (int i = 0; i < 4; i++)
                {
                    colors[i] = Color(pixels[i], sourceFormat);
                }

                // Horizontal interpolation
                Color top = Color::Lerp(colors[0], colors[1], fx);
                Color bottom = Color::Lerp(colors[2], colors[3], fx);

                // Vertical interpolation
                Color finalColor = Color::Lerp(top, bottom, fy);

                finalColor.ConvertTo(targetFormat, dstBuffer);
                break;
            }
            }

            // Get destination pixel location
            uint8_t *dstPixel = targetData +
                                dy * targetPitch +
                                dx * targetInfo.bytesPerPixel;

            // Handle blending
            if (bc.mode != BlendMode::NOBLEND)
            {
                context.GetBlendFunc()(dstPixel, dstBuffer, 1, targetInfo, sourceInfo, context.GetColoring(),false,bc);
            }
            else
            {
                MemHandler::MemCopy(dstPixel, dstBuffer, targetInfo.bytesPerPixel);
            }
        }
    }
}