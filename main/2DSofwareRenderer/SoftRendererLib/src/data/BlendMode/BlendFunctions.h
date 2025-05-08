#ifndef BLENDFUNCTIONS_H
#define BLENDFUNCTIONS_H

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"
#include <map>
#include <iostream>
#include "BlendMode.h"

namespace Tergos2D
{
    using BlendFunc = void (*)(uint8_t *dstRow,
                               const uint8_t *srcRow,
                               size_t rowLength,
                               const PixelFormatInfo &targetInfo,
                               const PixelFormatInfo &sourceInfo,
                               Coloring coloring,
                               bool useSolidColor,
                               BlendContext& context);

    class BlendFunctions
    {
    private:
        static BlendFunc GetBlendFunc(PixelFormat format, bool useSolidColor)
        {
            switch (format)
            {
            case PixelFormat::RGB24:
                if (useSolidColor)
                    return BlendSolidRowRGB24;
                return BlendRGB24;
            case PixelFormat::BGR24:
                if (useSolidColor)
                    return BlendSolidRowRGB24;
                return BlendRGB24;
            default:
                return nullptr;
            }
        }

    public:
        static void BlendRow(uint8_t *dstRow,
                             const uint8_t *srcRow,
                             size_t rowLength,
                             const PixelFormatInfo &targetInfo,
                             const PixelFormatInfo &sourceInfo,
                             Coloring coloring,
                             bool useSolidColor,
                             BlendContext& context);

        static void BlendRGB24(uint8_t *dstRow,
                               const uint8_t *srcRow,
                               size_t rowLength,
                               const PixelFormatInfo &targetInfo,
                               const PixelFormatInfo &sourceInfo,
                               Coloring coloring,
                               bool useSolidColor,
                               BlendContext& context);

        static void BlendRGB565(uint8_t *dstRow,
                                const uint8_t *srcRow,
                                size_t rowLength,
                                const PixelFormatInfo &targetInfo,
                                const PixelFormatInfo &sourceInfo,
                                Coloring coloring,
                                bool useSolidColor,
                                BlendContext& context);
                                
        // ONLY SOURCEALPHA, INVERSESOURCEALPHA, ADD ONE ZERO ADD
        static void BlendToRGB24Simple(uint8_t *dstRow,
            const uint8_t *srcRow,
            size_t rowLength,
            const PixelFormatInfo &targetInfo,
            const PixelFormatInfo &sourceInfo,
            Coloring coloring,
            bool useSolidColor,
            BlendContext& context);

        static void BlendRGBA32ToRGB24(uint8_t *dstRow,
                                       const uint8_t *srcRow,
                                       size_t rowLength,
                                       const PixelFormatInfo &targetInfo,
                                       const PixelFormatInfo &sourceInfo,
                                       Coloring coloring,
                                       bool useSolidColor,
                                       BlendContext& context);

        static void BlendSolidRowRGB24(uint8_t *dstRow,
                                       const uint8_t *srcRow,
                                       size_t rowLength,
                                       const PixelFormatInfo &targetInfo,
                                       const PixelFormatInfo &sourceInfo,
                                       Coloring coloring,
                                       bool useSolidColor,
                                       BlendContext& context);
    };
} // namespace Tergos2D

#endif // !BLENDFUNCTIONS_H