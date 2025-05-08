#include "PrimitivesRenderer.h"
#include <algorithm>
#include "../util/MemHandler.h"
#include "../data/BlendMode/BlendFunctions.h"

#include "../RenderContext2D.h"
#include <float.h>
#include <math.h>

using namespace Tergos2D;

PrimitivesRenderer::PrimitivesRenderer(RenderContext2D &context) : RendererBase(context)
{
}
void PrimitivesRenderer::DrawRect(Color color, int16_t x, int16_t y, uint16_t length, uint16_t height)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch(); // Get the pitch (bytes per row)

    if (x < 0)
    {
        length = length + x;
        x = 0;
    }
    if (y < 0)
    {
        height = height + y;
        y = 0;
    }

    auto clippingArea = context.GetClippingArea();

    uint16_t clipStartX = context.IsClippingEnabled() ? std::max(x, clippingArea.startX) : x;
    uint16_t clipStartY = context.IsClippingEnabled() ? std::max(y, clippingArea.startY) : y;
    uint16_t clipEndX = context.IsClippingEnabled() ? std::min(static_cast<int>(x + length), static_cast<int>(clippingArea.endX)) : x + length;
    uint16_t clipEndY = context.IsClippingEnabled() ? std::min(static_cast<int>(y + height), static_cast<int>(clippingArea.endY)) : y + height;

    // Restrict drawing within the texture bounds
    clipEndX = std::min(clipEndX, textureWidth);
    clipEndY = std::min(clipEndY, textureHeight);

    // If nothing to draw, return
    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
        return;

    // Calculate the number of bytes in a row
    size_t bytesPerRow = (clipEndX - clipStartX) * info.bytesPerPixel;

    BlendContext bc = context.GetBlendContext();

    if (color.GetAlpha() == 255)
        bc.mode = BlendMode::NOBLEND;
    uint8_t *dest = textureData + (clipStartY * pitch) + (clipStartX * info.bytesPerPixel);

    uint8_t pixelData[MAXBYTESPERPIXEL];
    uint8_t rowPixelData[MAXROWLENGTH * MAXBYTESPERPIXEL];

    switch (bc.mode)
    {
    case BlendMode::NOBLEND:
    {

        color.ConvertTo(format, pixelData);

        uint8_t singlePixelData[MAXBYTESPERPIXEL];
        MemHandler::MemCopy(singlePixelData, pixelData, info.bytesPerPixel);

        for (size_t byteIndex = 0; byteIndex < bytesPerRow; byteIndex += info.bytesPerPixel)
        {
            MemHandler::MemCopy(rowPixelData + byteIndex, singlePixelData, info.bytesPerPixel);
        }

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *rowDest = dest + (j - clipStartY) * pitch;

            MemHandler::MemCopy(rowDest, rowPixelData, bytesPerRow);
        }
        break;
    }
    default:
    {
        //Prepare data set
        for (size_t byteIndex = 0; byteIndex < bytesPerRow; byteIndex += 4)
        {
            MemHandler::MemCopy(rowPixelData + byteIndex, color.data, 4);
        }

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *rowDest = dest + (j - clipStartY) * pitch;
            size_t rowLength = (clipEndX - clipStartX); // Number of pixels per row
            PixelFormatInfo infosrcColor = PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888);
            context.GetBlendFunc()(rowDest, rowPixelData, rowLength, info, infosrcColor, context.GetColoring(),true,context.GetBlendContext());
        }
        break;
    }
    }
}

void PrimitivesRenderer::DrawLine(Color color, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    if (x0 == x1)
    {
        DrawRect(color, x0, y0, 1, y1 - y0);
        return;
    }
    if (y0 == y1)
    {
        DrawRect(color, x0, y0, x1 - x0, 1);
        return;
    }
    auto clippingArea = context.GetClippingArea();

    // Clip the line to the clipping area
    if (context.IsClippingEnabled())
    {
        // Cohen-Sutherland clipping algorithm
        auto clipCode = [](int16_t x, int16_t y, const ClippingArea &clip)
        {
            int code = 0;
            if (x < clip.startX)
                code |= 1;
            if (x > clip.endX)
                code |= 2;
            if (y < clip.startY)
                code |= 4;
            if (y > clip.endY)
                code |= 8;
            return code;
        };

        int code0 = clipCode(x0, y0, clippingArea);
        int code1 = clipCode(x1, y1, clippingArea);
        bool accept = false;

        while (true)
        {
            if (!(code0 | code1))
            {
                accept = true;
                break;
            }
            else if (code0 & code1)
            {
                return;
            }
            else
            {
                int codeOut = code0 ? code0 : code1;
                int16_t x, y;

                if (codeOut & 8)
                {
                    x = x0 + (x1 - x0) * (clippingArea.endY - y0) / (y1 - y0);
                    y = clippingArea.endY;
                }
                else if (codeOut & 4)
                {
                    x = x0 + (x1 - x0) * (clippingArea.startY - y0) / (y1 - y0);
                    y = clippingArea.startY;
                }
                else if (codeOut & 2)
                {
                    y = y0 + (y1 - y0) * (clippingArea.endX - x0) / (x1 - x0);
                    x = clippingArea.endX;
                }
                else
                {
                    y = y0 + (y1 - y0) * (clippingArea.startX - x0) / (x1 - x0);
                    x = clippingArea.startX;
                }

                if (codeOut == code0)
                {
                    x0 = x;
                    y0 = y;
                    code0 = clipCode(x0, y0, clippingArea);
                }
                else
                {
                    x1 = x;
                    y1 = y;
                    code1 = clipCode(x1, y1, clippingArea);
                }
            }
        }

        if (!accept)
        {
            return;
        }
    }

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch();

    int16_t dx = std::abs(x1 - x0);
    int16_t dy = std::abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    BlendContext bc = context.GetBlendContext();

    if(color.GetAlpha() == 255) bc.mode = BlendMode::NOBLEND;
    uint8_t pixelData[MAXBYTESPERPIXEL];
    color.ConvertTo(format, pixelData);

    while (true)
    {
        if (x0 >= 0 && x0 < textureWidth && y0 >= 0 && y0 < textureHeight)
        {
            uint8_t *targetPixel = textureData + (y0 * pitch) + (x0 * info.bytesPerPixel);
            switch (bc.mode)
            {
            case BlendMode::NOBLEND:
                MemHandler::MemCopy(targetPixel, pixelData, info.bytesPerPixel);
                break;
            default:
                context.GetBlendFunc()(targetPixel, pixelData, 1, info, PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888), context.GetColoring(),false,context.GetBlendContext());
                break;
            }
        }

        if (x0 == x1 && y0 == y1)
            break;

        int16_t e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}


void PrimitivesRenderer::DrawTransformedRect(Color color, uint16_t length, uint16_t height, const float transformationMatrix[3][3])
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    // Identify rotation angle from the transformation matrix
    float cosAngle = transformationMatrix[0][0];
    float sinAngle = transformationMatrix[1][0];


    float scaleX = std::sqrt(transformationMatrix[0][0] * transformationMatrix[0][0] +
        transformationMatrix[1][0] * transformationMatrix[1][0]);
    float scaleY = std::sqrt(transformationMatrix[0][1] * transformationMatrix[0][1] +
            transformationMatrix[1][1] * transformationMatrix[1][1]);

    // Extract rotation from scaled matrix by normalizing with scale
    float rotateCos = transformationMatrix[0][0] / scaleX;
    float rotateSin = transformationMatrix[1][0] / scaleX;
    
    // Calculate angle in radians and degrees
    float angleInRadians = std::atan2(rotateSin, rotateCos);
    float angleInDegrees = angleInRadians * (180.0f / 3.14159265358979323846f);

    if((int)std::round(angleInDegrees) % 90 == 0)
    {
        // Need to handle different angles correctly
        int angle = ((int)std::round(angleInDegrees) % 360 + 360) % 360; // normalize to 0-359
        switch(angle) {
            case 0:   // 0 degrees
                DrawRect(color, transformationMatrix[0][2], transformationMatrix[1][2],
                        length * scaleX, height * scaleY);
                break;
            case 90:  // 90 degrees
                DrawRect(color, transformationMatrix[0][2] - height * scaleY, transformationMatrix[1][2],
                        height * scaleY, length * scaleX);
                break;
            case 180: // 180 degrees
                DrawRect(color, transformationMatrix[0][2] - length * scaleX,
                        transformationMatrix[1][2] - height * scaleY,
                        length * scaleX, height * scaleY);
                break;
            case 270: // 270 degrees
                DrawRect(color, transformationMatrix[0][2], transformationMatrix[1][2] - length * scaleX,
                        height * scaleY, length * scaleX);
                break;
        }
        return;
    }



    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch();


    // Calculate the bounding box of the transformed rectangle
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();

    std::vector<std::pair<float, float>> corners = {
        {0, 0},
        {static_cast<float>(length), 0},
        {0, static_cast<float>(height)},
        {static_cast<float>(length), static_cast<float>(height)}
    };

    for (const auto& corner : corners)
    {
        float x = transformationMatrix[0][0] * corner.first + transformationMatrix[0][1] * corner.second + transformationMatrix[0][2];
        float y = transformationMatrix[1][0] * corner.first + transformationMatrix[1][1] * corner.second + transformationMatrix[1][2];

        if (x < minX) minX = x;
        if (y < minY) minY = y;
        if (x > maxX) maxX = x;
        if (y > maxY) maxY = y;
    }

    // Clamp the bounding box to the target texture's dimensions
    int16_t startX = std::max(static_cast<int16_t>(std::floor(minX)), static_cast<int16_t>(0));
    int16_t startY = std::max(static_cast<int16_t>(std::floor(minY)), static_cast<int16_t>(0));
    int16_t endX = std::min(static_cast<int16_t>(std::ceil(maxX)), static_cast<int16_t>(textureWidth));
    int16_t endY = std::min(static_cast<int16_t>(std::ceil(maxY)), static_cast<int16_t>(textureHeight));

    if (context.IsClippingEnabled())
    {
        auto clippingArea = context.GetClippingArea();
        startX = std::max(startX, static_cast<int16_t>(clippingArea.startX));
        startY = std::max(startY, static_cast<int16_t>(clippingArea.startY));
        endX = std::min(endX, static_cast<int16_t>(clippingArea.endX));
        endY = std::min(endY, static_cast<int16_t>(clippingArea.endY));
    }

    // Define the inverse transformation matrix
    float invMatrix[3][3];
    float det = transformationMatrix[0][0] * (transformationMatrix[1][1] * transformationMatrix[2][2] - transformationMatrix[1][2] * transformationMatrix[2][1]) -
                transformationMatrix[0][1] * (transformationMatrix[1][0] * transformationMatrix[2][2] - transformationMatrix[1][2] * transformationMatrix[2][0]) +
                transformationMatrix[0][2] * (transformationMatrix[1][0] * transformationMatrix[2][1] - transformationMatrix[1][1] * transformationMatrix[2][0]);

    if (det == 0.0f)
        return; // Transformation matrix is not invertible

    float invDet = 1.0f / det;

    // Calculate the inverse matrix
    invMatrix[0][0] = (transformationMatrix[1][1] * transformationMatrix[2][2] - transformationMatrix[1][2] * transformationMatrix[2][1]) * invDet;
    invMatrix[0][1] = (transformationMatrix[0][2] * transformationMatrix[2][1] - transformationMatrix[0][1] * transformationMatrix[2][2]) * invDet;
    invMatrix[0][2] = (transformationMatrix[0][1] * transformationMatrix[1][2] - transformationMatrix[0][2] * transformationMatrix[1][1]) * invDet;
    invMatrix[1][0] = (transformationMatrix[1][2] * transformationMatrix[2][0] - transformationMatrix[1][0] * transformationMatrix[2][2]) * invDet;
    invMatrix[1][1] = (transformationMatrix[0][0] * transformationMatrix[2][2] - transformationMatrix[0][2] * transformationMatrix[2][0]) * invDet;
    invMatrix[1][2] = (transformationMatrix[0][2] * transformationMatrix[1][0] - transformationMatrix[0][0] * transformationMatrix[1][2]) * invDet;
    invMatrix[2][0] = (transformationMatrix[1][0] * transformationMatrix[2][1] - transformationMatrix[1][1] * transformationMatrix[2][0]) * invDet;
    invMatrix[2][1] = (transformationMatrix[0][1] * transformationMatrix[2][0] - transformationMatrix[0][0] * transformationMatrix[2][1]) * invDet;
    invMatrix[2][2] = (transformationMatrix[0][0] * transformationMatrix[1][1] - transformationMatrix[0][1] * transformationMatrix[1][0]) * invDet;

    BlendContext bc = context.GetBlendContext();
    if (color.GetAlpha() == 255)
        bc.mode = BlendMode::NOBLEND;

    uint8_t pixelData[MAXBYTESPERPIXEL];
    color.ConvertTo(format, pixelData);

    // Iterate over the bounding box in the target texture
    for (int16_t y = startY; y < endY; ++y)
    {
        for (int16_t x = startX; x < endX; ++x)
        {
            // Apply the inverse transformation to find the corresponding source pixel
            float srcX = invMatrix[0][0] * x + invMatrix[0][1] * y + invMatrix[0][2];
            float srcY = invMatrix[1][0] * x + invMatrix[1][1] * y + invMatrix[1][2];

            // Check if the source pixel is within bounds
            if (srcX >= 0 && srcX < length && srcY >= 0 && srcY < height)
            {
                uint8_t *dest = textureData + y * pitch + x * info.bytesPerPixel;

                switch (bc.mode)
                {
                case BlendMode::NOBLEND:
                    MemHandler::MemCopy(dest, pixelData, info.bytesPerPixel);
                    break;
                default:
                    context.GetBlendFunc()(dest, pixelData, 1, info, PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888), context.GetColoring(), true, bc);
                    break;
                }
            }
        }
    }
}