#include "Color.h"
#include "PixelFormat/PixelFormatInfo.h"
#include "PixelFormat/PixelConverter.h"
#include "../util/MemHandler.h"

#include <algorithm>

using namespace Tergos2D;

Color::Color(uint8_t r, uint8_t g, uint8_t b)
{
    data[1] = r;
    data[2] = g;
    data[3] = b;
    data[0] = 255;
    format = PixelFormat::ARGB8888;
}

Color::Color(const uint8_t *pixel, PixelFormat format)
{
    if (pixel == nullptr)
    {
        format = PixelFormat::ARGB8888;
    }
    if (format == PixelFormat::ARGB8888)
    {
        MemHandler::MemCopy(data, pixel, 4);
    }
    else
    {
        // Convert the internal data to the target format
        PixelConverter::Convert(format, PixelFormat::ARGB8888, pixel, this->data);
    }
}

PixelFormat Color::GetFormat()
{
    return format;
}

uint8_t Color::GetAlpha()
{
    return data[0];
}

Color::Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    data[0] = a;
    data[1] = r;
    data[2] = g;
    data[3] = b;
    format = PixelFormat::ARGB8888;
}

Color::Color(uint8_t grayscale)
{
    data[0] = grayscale;
    data[1] = grayscale;
    data[2] = grayscale;
    data[3] = grayscale;
    format = PixelFormat::ARGB8888;
}

void Color::GetColor(PixelFormat targetFormat, uint8_t *outColor) const
{
    if (format == targetFormat)
    {
        std::memcpy(outColor, data, PixelFormatRegistry::GetInfo(format).bytesPerPixel);
    }
    else
    {
        // Convert the internal data to the target format
        ConvertTo(targetFormat, outColor);
    }
}

void Color::SetColor(PixelFormat inputFormat, const uint8_t *colorData)
{
    // Set the format to the input format
    format = inputFormat;

    // Get the bytes per pixel for the input format
    int bytes = PixelFormatRegistry::GetInfo(format).bytesPerPixel;

    // Copy the input color data into internal storage
    std::memcpy(data, colorData, bytes);
}

void Color::ConvertTo(PixelFormat targetFormat, uint8_t *outColor) const
{ // If the current format is the same as the target, just copy
    if (format == targetFormat)
    {
        std::memcpy(outColor, data, PixelFormatRegistry::GetInfo(format).bytesPerPixel);
        return;
    }

    PixelConverter::Convert(format, targetFormat, data, outColor);
}

Color Color::Lerp(const Color &a, const Color &b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return Color(
        static_cast<uint8_t>(a.data[0] + t * (b.data[0] - b.data[0])),
        static_cast<uint8_t>(a.data[1] + t * (b.data[1] - a.data[1])),
        static_cast<uint8_t>(a.data[2] + t * (b.data[2] - a.data[2])),
        static_cast<uint8_t>(a.data[3] + t * (b.data[3] - a.data[3])));
}