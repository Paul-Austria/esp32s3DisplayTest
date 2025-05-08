#ifndef PIXELFORMATINFO_H
#define PIXELFORMATINFO_H

#include "PixelFormat.h"
#include <unordered_map>
#include "../Color.h"
#include <stdint.h>
namespace Tergos2D
{

    struct PixelFormatInfo
    {
        PixelFormat format;    // The pixel format
        uint8_t bytesPerPixel; // Number of bytes per pixel
        uint8_t bitsPerPixel;  // how many bits per pixel
        bool isBitFormat;      // uses less then one byte per pixel (grayscale 4 or grayscale 1)
        uint8_t numChannels;   // Number of color channels
        bool hasAlpha;         // Whether the format includes an alpha channel
        const char *name;      // A human-readable name for the format

        // Bit masks and shifts for each channel
        uint16_t redMask, greenMask, blueMask, alphaMask;
        uint8_t redShift, greenShift, blueShift, alphaShift;

        PixelFormatInfo() = default;

        PixelFormatInfo(PixelFormat format, uint8_t bpp, uint8_t bitspp, bool isBitFormat, uint8_t channels, bool alpha, const char *name,
                        uint16_t redMask, uint8_t redShift,
                        uint16_t greenMask, uint8_t greenShift,
                        uint16_t blueMask, uint8_t blueShift,
                        uint16_t alphaMask = 0, uint8_t alphaShift = 0)
            : format(format),
            bytesPerPixel(bpp),
            bitsPerPixel(bitspp),
            isBitFormat(isBitFormat),
            numChannels(channels),
            hasAlpha(alpha),
            name(name),
            redMask(redMask),
            greenMask(greenMask),
            blueMask(blueMask),
            alphaMask(alphaMask),
            redShift(redShift),
            greenShift(greenShift),
            blueShift(blueShift),
            alphaShift(alphaShift)
        {
            if (isBitFormat)
            {
                bytesPerPixel = bitsPerPixel % 8;
            }
            else
            {
                bitsPerPixel = bytesPerPixel * 8;
            }
        }

    private:
    };

    class PixelFormatRegistry
    {
    public:
        static const PixelFormatInfo &GetInfo(PixelFormat format);

    private:
        static const PixelFormatInfo formatInfoArray[];
    };

}

#endif // PIXELFORMATINFO_H
