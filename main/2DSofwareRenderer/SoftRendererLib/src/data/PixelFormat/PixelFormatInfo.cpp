#include "PixelFormatInfo.h"

namespace Tergos2D
{
    const PixelFormatInfo PixelFormatRegistry::formatInfoArray[] = {
     {PixelFormat::RGB24, 3, 0, false, 3, false, "RGB24", 0xFF, 0, 0xFF, 8, 0xFF, 16},
       {PixelFormat::BGR24, 3, 0, false, 3, false, "BGR24", 0xFF, 16, 0xFF, 8, 0xFF, 0},

        {PixelFormat::ARGB8888, 4, 0, false, 4, true, "ARGB8888", 0xFF, 16, 0xFF, 8, 0xFF, 0, 0xFF, 0},
         {PixelFormat::RGBA8888, 4, 0, false, 4, true, "RGBA8888", 0xFF, 24, 0xFF, 16, 0xFF, 8, 0xFF, 24},
        {PixelFormat::ARGB1555, 2, 0, false, 4, true, "RGBA1555", 0x7C00, 10, 0x03E0, 5, 0x001F, 0, 0x8000, 15},
      {PixelFormat::RGB565, 2, 0, false, 3, false, "RGB565", 0xF800, 11, 0x07E0, 5, 0x001F, 0},
        {PixelFormat::RGBA4444, 2, 0, false, 4, true, "RGBA4444", 0xF000, 12, 0x0F00, 8, 0x00F0, 4, 0x000F, 0},
        {PixelFormat::GRAYSCALE8, 1, 0, false, 1, true, "Grayscale8", 0xFF, 0, 0x00, 0, 0x00, 0},
    };

    const PixelFormatInfo &PixelFormatRegistry::GetInfo(PixelFormat format)
    {
        return formatInfoArray[static_cast<int>(format)];
    }
}
