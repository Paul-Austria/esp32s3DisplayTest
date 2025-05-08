
#include "TransformedTextureRenderer.h"
#include <algorithm>
#include "../../util/MemHandler.h"
#include "../../data/BlendMode/BlendFunctions.h"
#include "../../data/PixelFormat/PixelConverter.h"
#include "../RenderContext2D.h"
#include <float.h>
#include <math.h>
#include <cstdio>
using namespace Tergos2D;

TransformedTextureRenderer::TransformedTextureRenderer(RenderContext2D &context) : RendererBase(context)
{
}


void TransformedTextureRenderer::DrawTexture(Texture &texture, const float transformationMatrix[3][3], int startX, int StartY, int endX, int endY)
{
    if(m_drawTexture == nullptr) return;
    if(startX == 0 && StartY == 0 && endX == 0 && endY == 0)
    {
        endX = texture.GetWidth();
        endY = texture.GetHeight();
    }
    m_drawTexture(texture,transformationMatrix, context,startX,StartY,endX,endY);
}
void Tergos2D::TransformedTextureRenderer::DrawTexture(Texture &texture, const float transformationMatrix[3][3], RenderContext2D &context, int tstartX, int tStartY, int tendX, int tendY)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture || !texture.GetData())
    {
        return;
    }

     // Get texture information
     PixelFormat sourceFormat = texture.GetFormat();
     PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
     uint8_t *sourceData = texture.GetData();
     uint16_t sourceWidth = texture.GetWidth();
     uint16_t sourceHeight = texture.GetHeight();
     size_t sourcePitch = texture.GetPitch();

     PixelFormat targetFormat = targetTexture->GetFormat();
     PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
     uint8_t *targetData = targetTexture->GetData();
     uint16_t targetWidth = targetTexture->GetWidth();
     uint16_t targetHeight = targetTexture->GetHeight();
     size_t targetPitch = targetTexture->GetPitch();

     BlendContext bc = context.GetBlendContext();
     bc.mode = context.BlendModeToUse(sourceInfo);

    const float EPSILON = 0.0001f;

    // Extract scale and rotation
    float scaleX = std::sqrt(transformationMatrix[0][0] * transformationMatrix[0][0] +
                            transformationMatrix[1][0] * transformationMatrix[1][0]);
    float scaleY = std::sqrt(transformationMatrix[0][1] * transformationMatrix[0][1] +
                            transformationMatrix[1][1] * transformationMatrix[1][1]);

    // Check if there's no scaling (approximately 1.0)
    bool noScaling = std::abs(scaleX - 1.0f) < EPSILON && std::abs(scaleY - 1.0f) < EPSILON;

    // Check for perspective transformation
    bool noPerspective = std::abs(transformationMatrix[2][0]) < EPSILON &&
                        std::abs(transformationMatrix[2][1]) < EPSILON &&
                        std::abs(transformationMatrix[2][2] - 1.0f) < EPSILON;

    if (noScaling && noPerspective)
    {
        // Extract rotation
        float rotateCos = transformationMatrix[0][0];
        float rotateSin = transformationMatrix[1][0];
        float angleInRadians = std::atan2(rotateSin, rotateCos);
        float angleInDegrees = angleInRadians * (180.0f / 3.14159265358979323846f);
        int angle = ((int)std::round(angleInDegrees) % 360 + 360) % 360;

        if (angle % 90 == 0)
        {

            PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);
            if (!convertFunc) return;

            // Calculate destination coordinates based on rotation
            int16_t destX = static_cast<int16_t>(transformationMatrix[0][2]);
            int16_t destY = static_cast<int16_t>(transformationMatrix[1][2]);

            // Adjust clipping based on rotation
            int16_t startX = destX;
            int16_t startY = destY;
            int16_t width = sourceWidth;
            int16_t height = sourceHeight;

            switch (angle)
            {
                case 0:   // 0 degrees
                    context.basicTextureRenderer.DrawTexture(texture,destX,destY);
                    return;
                    break;
                case 90:  // 90 degrees
                    startX = destX - sourceHeight;
                    std::swap(width, height);
                    break;
                case 180: // 180 degrees
                    startX = destX - sourceWidth;
                    startY = destY - sourceHeight;
                    break;
                case 270: // 270 degrees
                    startY = destY - sourceWidth;
                    std::swap(width, height);
                    break;
            }

            // Apply clipping
            if (context.IsClippingEnabled())
            {
                auto clippingArea = context.GetClippingArea();
                startX = std::max(startX, static_cast<int16_t>(clippingArea.startX));
                startY = std::max(startY, static_cast<int16_t>(clippingArea.startY));
                width = std::min(width, static_cast<int16_t>(clippingArea.endX - startX));
                height = std::min(height, static_cast<int16_t>(clippingArea.endY - startY));
            }

            // Optimized copy for each rotation
            const int maxPos = MAX_BUFFER_SIZE;
            uint8_t buffer[maxPos*4];  // Reuse the buffer from original code
            int pos = 0;
            uint8_t* targetPixel = nullptr;
            for (int16_t y = 0; y < height; ++y)
            {
                for (int16_t x = 0; x < width; ++x)
                {
                    int sourceX, sourceY;
                    switch (angle)
                    {
                        case 0:
                            sourceX = x;
                            sourceY = y;
                            break;
                        case 270:
                            sourceX = height - 1 - y;
                            sourceY = x;
                            break;
                        case 180:
                            sourceX = width - 1 - x;
                            sourceY = height - 1 - y;
                            break;
                        case 90:
                            sourceX = y;
                            sourceY = width - 1 - x;
                            break;
                    }

                    // Check texture bounds
                    if (sourceX < tstartX || sourceX > tendX || sourceY < tStartY || sourceY > tendY)
                        continue;

                    const uint8_t *sourcePixel = sourceData + sourceY * sourcePitch + sourceX * sourceInfo.bytesPerPixel;
                    if (pos == 0)
                    {
                        targetPixel = targetData + (startY + y) * targetPitch + (startX + x) * targetInfo.bytesPerPixel;
                    }

                    std::memcpy(buffer + sourceInfo.bytesPerPixel * pos, sourcePixel, sourceInfo.bytesPerPixel);
                    pos++;

                    if (pos == maxPos)
                    {
                        if (bc.mode == BlendMode::NOBLEND)
                        {
                            convertFunc(buffer, targetPixel, pos);
                        }
                        else
                        {
                            context.GetBlendFunc()(targetPixel, buffer, pos, targetInfo, sourceInfo, context.GetColoring(), false, bc);
                        }
                        pos = 0;
                    }
                }

                // Handle remaining pixels in the row
                if (pos != 0)
                {
                    if (bc.mode == BlendMode::NOBLEND)
                    {
                        convertFunc(buffer, targetPixel, pos);
                    }
                    else
                    {
                        context.GetBlendFunc()(targetPixel, buffer, pos, targetInfo, sourceInfo, context.GetColoring(), false, bc);
                    }
                    pos = 0;
                }
            }
            return;
        }
    }

    // Calculate the bounding box of the transformed source texture
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();

    std::vector<std::pair<float, float>> corners = {
        {0, 0},
        {static_cast<float>(sourceWidth), 0},
        {0, static_cast<float>(sourceHeight)},
        {static_cast<float>(sourceWidth), static_cast<float>(sourceHeight)}
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
    int16_t endX = std::min(static_cast<int16_t>(std::ceil(maxX)), static_cast<int16_t>(targetWidth));
    int16_t endY = std::min(static_cast<int16_t>(std::ceil(maxY)), static_cast<int16_t>(targetHeight));


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


    // Iterate over the bounding box in the target texture
    const int maxPos = MAX_BUFFER_SIZE;
    uint8_t buffer[maxPos*4];
    int pos = 0;

    uint8_t *targetPixel = nullptr;
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);
    if(!convertFunc) return;
    for (int16_t y = startY; y < endY; ++y)
    {
        for (int16_t x = startX; x < endX; ++x)
        {
            // Apply the inverse transformation to find the corresponding source pixel
            float srcX = invMatrix[0][0] * x + invMatrix[0][1] * y + invMatrix[0][2];
            float srcY = invMatrix[1][0] * x + invMatrix[1][1] * y + invMatrix[1][2];

            // in case we don't want to render the entire texture we need to clip it again
            if (srcX < tstartX || srcX > tendX || srcY < tStartY || srcY > tendY)
                continue;
            // Check if the source pixel is within bounds
            if (srcX >= 0 && srcX < sourceWidth && srcY >= 0 && srcY < sourceHeight)
            {
                uint16_t intSrcX = static_cast<uint16_t>(srcX);
                uint16_t intSrcY = static_cast<uint16_t>(srcY);
                if(pos == 0)
                {
                    targetPixel = targetData + y * targetPitch + x * targetInfo.bytesPerPixel;
                }
                const uint8_t *sourcePixel = sourceData + intSrcY * sourcePitch + intSrcX * sourceInfo.bytesPerPixel;
                std::memcpy(buffer + sourceInfo.bytesPerPixel*pos, sourcePixel,sourceInfo.bytesPerPixel);
                pos++;

                if (pos == maxPos)
                {

                    if(bc.mode == BlendMode::NOBLEND){
                        convertFunc(buffer, targetPixel, pos);
                    }
                    else{
                        context.GetBlendFunc()(targetPixel, buffer, pos, targetInfo, sourceInfo, context.GetColoring(),false,bc);
                    }
                    pos = 0;
                }

            }
        }
        if(pos != 0)
        {
            if(bc.mode == BlendMode::NOBLEND){
                convertFunc(buffer, targetPixel, pos);
            }
            else{
                context.GetBlendFunc()(targetPixel, buffer, pos, targetInfo, sourceInfo, context.GetColoring(),false,bc);
            }
            pos = 0;
        }
    }
}

void Tergos2D::TransformedTextureRenderer::DrawTextureSamplingSupp(Texture &texture, const float transformationMatrix[3][3], RenderContext2D &context, int tstartX, int tStartY, int tendX, int tendY)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture || !texture.GetData())
    {
        return;
    }
    //revert to normal method when nearest is used
    if(context.GetSamplingMethod() == SamplingMethod::NEAREST){
        DrawTexture(texture,transformationMatrix,context,tstartX,tStartY,tendX,tendY);
        return;
    }

    const float EPSILON = 0.0001f;

    // Extract scale and rotation
    float scaleX = std::sqrt(transformationMatrix[0][0] * transformationMatrix[0][0] +
                            transformationMatrix[1][0] * transformationMatrix[1][0]);
    float scaleY = std::sqrt(transformationMatrix[0][1] * transformationMatrix[0][1] +
                            transformationMatrix[1][1] * transformationMatrix[1][1]);

    // Check if there's no scaling (approximately 1.0)
    bool noScaling = std::abs(scaleX - 1.0f) < EPSILON && std::abs(scaleY - 1.0f) < EPSILON;

    // Check for perspective transformation
    bool noPerspective = std::abs(transformationMatrix[2][0]) < EPSILON &&
                        std::abs(transformationMatrix[2][1]) < EPSILON &&
                        std::abs(transformationMatrix[2][2] - 1.0f) < EPSILON;

    if (noScaling && noPerspective)
    {
        // Extract rotation
        float rotateCos = transformationMatrix[0][0];
        float rotateSin = transformationMatrix[1][0];
        float angleInRadians = std::atan2(rotateSin, rotateCos);
        float angleInDegrees = angleInRadians * (180.0f / 3.14159265358979323846f);
        int angle = ((int)std::round(angleInDegrees) % 360 + 360) % 360;
        // No sampling needed if 90 degrees
        if (angle % 90 == 0)
        {
            DrawTexture(texture,transformationMatrix,context,tstartX,tStartY,tendX,tendY);
            return;
        }
    }

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch();

    // Get target texture information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Calculate the bounding box of the transformed source texture
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();

    std::vector<std::pair<float, float>> corners = {
        {0, 0},
        {static_cast<float>(sourceWidth), 0},
        {0, static_cast<float>(sourceHeight)},
        {static_cast<float>(sourceWidth), static_cast<float>(sourceHeight)}
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
    int16_t endX = std::min(static_cast<int16_t>(std::ceil(maxX)), static_cast<int16_t>(targetWidth));
    int16_t endY = std::min(static_cast<int16_t>(std::ceil(maxY)), static_cast<int16_t>(targetHeight));


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
    bc.mode = context.BlendModeToUse(sourceInfo);

    // Iterate over the bounding box in the target texture
    const int maxPos = MAX_BUFFER_SIZE;
    uint8_t buffer[maxPos*4];
    int pos = 0;

    uint8_t *targetPixel = nullptr;
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);
    if(!convertFunc) return;
    auto sampMethod = context.GetSamplingMethod();
    for (int16_t y = startY; y < endY; ++y)
    {
        for (int16_t x = startX; x < endX; ++x)
        {
            // Apply the inverse transformation to find the corresponding source pixel
            float srcX = invMatrix[0][0] * x + invMatrix[0][1] * y + invMatrix[0][2];
            float srcY = invMatrix[1][0] * x + invMatrix[1][1] * y + invMatrix[1][2];


            // in case we don't want to render the entire texture we need to clip it again
            if (srcX < tstartX || srcX > tendX || srcY < tStartY || srcY > tendY)
                continue;


            // Check if the source pixel is within bounds
            if (srcX >= 0 && srcX < sourceWidth && srcY >= 0 && srcY < sourceHeight)
            {
                uint16_t intSrcX = static_cast<uint16_t>(srcX);
                uint16_t intSrcY = static_cast<uint16_t>(srcY);
                if(pos == 0)
                {
                    targetPixel = targetData + y * targetPitch + x * targetInfo.bytesPerPixel;
                }
                const uint8_t *sourcePixel = sourceData + intSrcY * sourcePitch + intSrcX * sourceInfo.bytesPerPixel;

                switch (sampMethod)
                {
                case SamplingMethod::LINEAR:
                    {
                        // Compute the weights for linear interpolation
                        float fracX = srcX - intSrcX;
                        float fracY = srcY - intSrcY;

                        uint8_t *srcPixel00 = sourceData + intSrcY * sourcePitch + intSrcX * sourceInfo.bytesPerPixel;
                        uint8_t *srcPixel01 = sourceData + intSrcY * sourcePitch + (intSrcX + 1) * sourceInfo.bytesPerPixel;
                        uint8_t *srcPixel10 = sourceData + (intSrcY + 1) * sourcePitch + intSrcX * sourceInfo.bytesPerPixel;
                        uint8_t *srcPixel11 = sourceData + (intSrcY + 1) * sourcePitch + (intSrcX + 1) * sourceInfo.bytesPerPixel;

                        // Linear interpolation
                        for (int c = 0; c < sourceInfo.bytesPerPixel; ++c)
                        {
                            float value = (1 - fracX) * (1 - fracY) * srcPixel00[c] +
                                            fracX * (1 - fracY) * srcPixel01[c] +
                                            (1 - fracX) * fracY * srcPixel10[c] +
                                            fracX * fracY * srcPixel11[c];

                            buffer[pos * sourceInfo.bytesPerPixel + c] = static_cast<uint8_t>(value);
                        }
                        break;
                    }
                default:
                        std::memcpy(buffer + sourceInfo.bytesPerPixel*pos, sourcePixel,sourceInfo.bytesPerPixel);
                    break;
                }

                pos++;

                if (pos == maxPos)
                {
                    if(bc.mode == BlendMode::NOBLEND){
                        convertFunc(buffer, targetPixel, pos);
                    }
                    else{
                        context.GetBlendFunc()(targetPixel, buffer, pos, targetInfo, sourceInfo, context.GetColoring(),false,bc);
                    }
                    pos = 0;
                }

            }
        }
        if(pos != 0)
        {
            if(bc.mode == BlendMode::NOBLEND){
                convertFunc(buffer, targetPixel, pos);
            }
            else{
                context.GetBlendFunc()(targetPixel, buffer, pos, targetInfo, sourceInfo, context.GetColoring(),false,bc);
            }
            pos = 0;
        }
    }
}


DrawTexturePointer Tergos2D::TransformedTextureRenderer::GetDrawTexture()
{
    return m_drawTexture;
}

void Tergos2D::TransformedTextureRenderer::SetDrawTexture(DrawTexturePointer drawTexture)
{
    if(drawTexture == nullptr) return;
    this->m_drawTexture = drawTexture;
}
