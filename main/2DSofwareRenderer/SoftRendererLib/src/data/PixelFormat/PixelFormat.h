#ifndef PIXELFORMAT_H
#define PIXELFORMAT_H

namespace Tergos2D
{

    enum class PixelFormat
    {
        RGB24, // 8 bits per channel: R, G, B or RGB888
        BGR24,
        ARGB8888,
        RGBA8888,
        ARGB1555, // 16 bits: 5 bits R, 5 bits G, 5 bits B, 1 bit A
        RGB565, // 16 bits: 5 bits R, 6 bits G, 5 bits B
        RGBA4444,
        GRAYSCALE8, // 8 bits grayscale

    };

}

#endif //  PIXELFORMAT_H
