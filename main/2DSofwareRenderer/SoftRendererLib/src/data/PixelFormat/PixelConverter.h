#ifndef PIXELCONVERTER_H
#define PIXELCONVERTER_H

#include "PixelFormat.h"
#include <cstring>
#include <stdexcept>
#include <cstdint>

namespace Tergos2D
{
    class PixelConverter
    {
    public:
        using ConvertFunc = void (*)(const uint8_t *src, uint8_t *dst, size_t count);

        // Get the conversion function from one format to another
        static ConvertFunc GetConversionFunction(PixelFormat from, PixelFormat to);

        // Convert pixels using cached function pointer and batch processing
        static void Convert(PixelFormat from, PixelFormat to, const uint8_t *src, uint8_t *dst, size_t count = 1);

    private:
        struct Conversion
        {
            PixelFormat from;
            PixelFormat to;
            ConvertFunc func;
        };

        static void Move(const uint8_t *src, uint8_t *dst, size_t count);
        static void Move2(const uint8_t *src, uint8_t *dst, size_t count);
        static void Move3(const uint8_t *src, uint8_t *dst, size_t count);
        static void Move4(const uint8_t *src, uint8_t *dst, size_t count);

        // BGR24 Conversions
        static void BGR24ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void BGR24ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void BGR24ToRGB565(const uint8_t *src, uint8_t *dst, size_t count);

        // RGB24 conversions
        static void RGB24ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB24ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB24ToBGR24(const uint8_t *src, uint8_t *dst, size_t count);
        static void BGR24ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB24ToRGB565(const uint8_t *src, uint8_t *dst, size_t count);

        static void RGB24ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count);

        // ARGB8888 conversions
        static void ARGB8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToBGR24(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToRGB565(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToARGB1555(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToRGBA4444(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);

        // RGBA8888 conversions
        static void RGBA8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA8888ToRGB565(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA8888ToBGR24(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA8888ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);

        // RGB565 conversions
        static void RGB565ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB565ToBGR24(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB565ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB565ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB565ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count);

        // ARGB1555 conversions
        static void ARGB1555ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB1555ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB1555ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB1555ToBGR24(const uint8_t *src, uint8_t *dst, size_t count);

        // RGBA4444 conversions
        static void RGBA4444ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA4444ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);

        // Grayscale conversions
        static void Grayscale8ToBGR24(const uint8_t *src, uint8_t *dst, size_t count);
        static void Grayscale8ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);
        static void Grayscale8ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void Grayscale8ToRGB565(const uint8_t *src, uint8_t *dst, size_t count);


        // Conversion mappings
        static constexpr Conversion defaultConversions[] = {
            // BGR24 conversions
            {PixelFormat::BGR24, PixelFormat::ARGB8888, BGR24ToARGB8888},
            {PixelFormat::BGR24, PixelFormat::RGBA8888, BGR24ToRGBA8888},
            {PixelFormat::BGR24, PixelFormat::RGB24, BGR24ToRGB24},
            {PixelFormat::BGR24, PixelFormat::RGB565, BGR24ToRGB565},

            // RGB24 conversions
            {PixelFormat::RGB24, PixelFormat::ARGB8888, RGB24ToARGB8888},
            {PixelFormat::RGB24, PixelFormat::RGBA8888, RGB24ToRGBA8888},
            {PixelFormat::RGB24, PixelFormat::GRAYSCALE8, RGB24ToGrayscale8},
            {PixelFormat::RGB24, PixelFormat::BGR24, RGB24ToBGR24},
            {PixelFormat::RGB24, PixelFormat::RGB565, RGB24ToRGB565},

            // ARGB8888 conversions
            {PixelFormat::ARGB8888, PixelFormat::RGB24, ARGB8888ToRGB24},
            {PixelFormat::ARGB8888, PixelFormat::BGR24, ARGB8888ToBGR24},

            {PixelFormat::ARGB8888, PixelFormat::RGB565, ARGB8888ToRGB565},
            {PixelFormat::ARGB8888, PixelFormat::ARGB1555, ARGB8888ToARGB1555},
            {PixelFormat::ARGB8888, PixelFormat::RGBA4444, ARGB8888ToRGBA4444},
            {PixelFormat::ARGB8888, PixelFormat::GRAYSCALE8, ARGB8888ToGrayscale8},
            {PixelFormat::ARGB8888, PixelFormat::RGBA8888, ARGB8888ToRGBA8888},

            // RGBA8888 conversions
            {PixelFormat::RGBA8888, PixelFormat::RGB24, RGBA8888ToRGB24},
            {PixelFormat::RGBA8888, PixelFormat::BGR24, RGBA8888ToBGR24},
            {PixelFormat::RGBA8888, PixelFormat::RGB565, RGBA8888ToRGB565},
            {PixelFormat::RGBA8888, PixelFormat::ARGB8888, RGBA8888ToARGB8888},

            // RGB565 conversions
            {PixelFormat::RGB565, PixelFormat::RGB24, RGB565ToRGB24},
            {PixelFormat::RGB565, PixelFormat::BGR24, RGB565ToBGR24},
            {PixelFormat::RGB565, PixelFormat::ARGB8888, RGB565ToARGB8888},
            {PixelFormat::RGB565, PixelFormat::RGBA8888, RGB565ToRGBA8888},
            {PixelFormat::RGB565, PixelFormat::GRAYSCALE8, RGB565ToGrayscale8},

            // ARGB1555 conversions
            {PixelFormat::ARGB1555, PixelFormat::ARGB8888, ARGB1555ToARGB8888},
            {PixelFormat::ARGB1555, PixelFormat::RGBA8888, ARGB1555ToRGBA8888},
            {PixelFormat::ARGB1555, PixelFormat::RGB24, ARGB1555ToRGB24},
            {PixelFormat::ARGB1555, PixelFormat::BGR24, ARGB1555ToBGR24},

            // RGBA4444 conversions
            {PixelFormat::RGBA4444, PixelFormat::ARGB8888, RGBA4444ToARGB8888},
            {PixelFormat::RGBA4444, PixelFormat::RGBA8888, RGBA4444ToRGBA8888},

            // Grayscale conversions
            {PixelFormat::GRAYSCALE8, PixelFormat::BGR24, Grayscale8ToBGR24},
            {PixelFormat::GRAYSCALE8, PixelFormat::RGB24, Grayscale8ToRGB24},
            {PixelFormat::GRAYSCALE8, PixelFormat::ARGB8888,Grayscale8ToARGB8888},
            {PixelFormat::GRAYSCALE8, PixelFormat::RGB565,Grayscale8ToRGB565},

        };
    };
}

#endif // PIXELCONVERTER_H
