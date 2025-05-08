#include "RenderContext2D.h"
#include "../data/PixelFormat/PixelConverter.h"
#include "../data/PixelFormat/PixelFormatInfo.h"
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include "../util/MemHandler.h"
#include "../data/BlendMode/BlendFunctions.h"

using namespace Tergos2D;



RenderContext2D::RenderContext2D() : primitivesRenderer(*this), basicTextureRenderer(*this), transformedTextureRenderer(*this),scaleTextureRenderer(*this)
{
}
void RenderContext2D::SetTargetTexture(Texture *targettexture)
{
    this->targetTexture = targettexture;
}

Texture * RenderContext2D::GetTargetTexture()
{
    return targetTexture;
}



BlendMode Tergos2D::RenderContext2D::BlendModeToUse(const PixelFormatInfo &info)
{
    BlendMode touse = m_BlendContext.mode;

    if(touse == BlendMode::NOBLEND) return touse;
    if(!info.hasAlpha && !GetColoring().colorEnabled) return BlendMode::NOBLEND;
    if(!info.hasAlpha && GetColoring().colorEnabled) return BlendMode::COLORINGONLY;

    return touse;
}

void Tergos2D::RenderContext2D::SetSamplingMethod(SamplingMethod method)
{
    this->samplingMethod = method;
}

SamplingMethod Tergos2D::RenderContext2D::GetSamplingMethod()
{
    return samplingMethod;
}


void RenderContext2D::ClearTarget(Color color)
{
    if (targetTexture == nullptr)
    {
        return;
    }

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t width = targetTexture->GetWidth();
    uint16_t height = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch();

    uint8_t pixelData[4];
    color.ConvertTo(format, pixelData);

    // Fill the first row with the pixel data
    uint8_t *row = textureData;
    for (uint32_t x = 0; x < width; ++x)
    {
        MemHandler::MemCopy(row + x * info.bytesPerPixel, pixelData, info.bytesPerPixel);
    }

    // Copy the first row to the rest of the rows
    for (uint32_t y = 1; y < height; ++y)
    {
        MemHandler::MemCopy(textureData + y * pitch, textureData, width * info.bytesPerPixel);
    }
}


void RenderContext2D::EnableClipping(bool clipping)
{
    this->enableClipping = clipping;
}
bool Tergos2D::RenderContext2D::IsClippingEnabled()
{
    return enableClipping;
}
void RenderContext2D::SetClipping(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    this->clippingArea.startX = startX;
    this->clippingArea.startY = startY;
    this->clippingArea.endX = endX;
    this->clippingArea.endY = endY;
}

ClippingArea Tergos2D::RenderContext2D::GetClippingArea()
{
    return clippingArea;
}

void Tergos2D::RenderContext2D::SetColoringSettings(Coloring coloring)
{
    this->colorOverlay = coloring;
}

Coloring &Tergos2D::RenderContext2D::GetColoring()
{
    return colorOverlay;
}

void Tergos2D::RenderContext2D::SetBlendFunc(BlendFunc blendFunc)
{
    if(blendFunc != nullptr)
        this->blendFunc = blendFunc;
}

BlendFunc Tergos2D::RenderContext2D::GetBlendFunc()
{
    return blendFunc;
}

BlendContext &Tergos2D::RenderContext2D::GetBlendContext()
{
    return m_BlendContext;
}

void Tergos2D::RenderContext2D::SetBlendContext(BlendContext context)
{
    this->m_BlendContext = context;
}
