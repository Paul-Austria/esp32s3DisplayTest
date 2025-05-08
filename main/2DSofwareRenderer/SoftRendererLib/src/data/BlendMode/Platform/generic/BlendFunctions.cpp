#include "../../BlendMode.h"
#include "../../BlendFunctions.h"
#include "../../../PixelFormat/PixelConverter.h"
#include "../../../PixelFormat/PixelFormatInfo.h"

using namespace Tergos2D;


// ONLY SOURCEALPHA, INVERSESOURCEALPHA, ADD ONE ZERO ADD
void BlendFunctions::BlendToRGB24Simple(uint8_t *dstRow,
    const uint8_t *srcRow,
    size_t rowLength,
    const PixelFormatInfo &targetInfo,
    const PixelFormatInfo &sourceInfo,
    Coloring coloring,
    bool useSolidColor,
    BlendContext& context)
{
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);

    const uint8_t *srcPixel = srcRow;
    uint8_t *dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled * coloring.color.data[0];
    uint8_t inverseColorFactor = 255 - colorFactor;

    for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, dstPixel += targetInfo.bytesPerPixel)
    {
        uint8_t grayValue = srcPixel[0];
        uint8_t alpha = (grayValue == 0) ? 0 : 255;

        if (alpha == 0)
        {
        continue;
        }

        uint8_t *srcColor = &srcRGB24[i * 3];

        // Apply coloring if needed
        if (coloring.colorEnabled)
        {
            srcColor[0] = (srcColor[0] * colorDataAsRGB[0]) >> 8;
            srcColor[1] = (srcColor[1] * colorDataAsRGB[1]) >> 8;
            srcColor[2] = (srcColor[2] * colorDataAsRGB[2]) >> 8;
            alpha = (alpha * coloring.color.data[0]) >> 8;
        }

        uint8_t invAlpha = 255 - alpha;

        // Blend the source and destination pixels
        dstPixel[0] = (srcColor[0] * alpha + dstPixel[0] * invAlpha) >> 8;
        dstPixel[1] = (srcColor[1] * alpha + dstPixel[1] * invAlpha) >> 8;
        dstPixel[2] = (srcColor[2] * alpha + dstPixel[2] * invAlpha) >> 8;
    }
}


void BlendFunctions::BlendRGB565(uint8_t *dstRow,
                                 const uint8_t *srcRow,
                                 size_t rowLength,
                                 const PixelFormatInfo &targetInfo,
                                 const PixelFormatInfo &sourceInfo,
                                 Coloring coloring,
                                 bool useSolidColor,
                                 BlendContext& context)
        {
        PixelConverter::ConvertFunc convertToRGB565 = PixelConverter::GetConversionFunction(sourceInfo.format, PixelFormat::RGB565);
        PixelConverter::ConvertFunc convertColorToRGB565 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, PixelFormat::RGB565);

        // Temporary storage for source pixel in RGB565
        alignas(16) uint16_t srcRGB565[1024];
        alignas(16) uint16_t colorDataAsRGB565;

        convertToRGB565(srcRow, reinterpret_cast<uint8_t*>(srcRGB565), rowLength);

        const uint8_t *srcPixel = srcRow;
        uint16_t *dstPixel = reinterpret_cast<uint16_t*>(dstRow);

        uint8_t colorFactor = coloring.colorEnabled * coloring.color.data[0];
        uint8_t inverseColorFactor = 255 - colorFactor;

        for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, ++dstPixel)
        {
            uint8_t grayValue = srcPixel[0];
            uint8_t alpha = (grayValue == 0) ? 0 : 255;

            if (alpha == 0)
            {
                continue;
            }

            uint16_t srcColor = srcRGB565[i];

            // Apply coloring if needed
            if (coloring.colorEnabled)
            {
                // Extract RGB components with proper scaling
                uint32_t r = (srcColor >> 11) & 0x1F;
                uint32_t g = (srcColor >> 5) & 0x3F;
                uint32_t b = srcColor & 0x1F;

                // Scale up for better precision
                r = (r << 3) | (r >> 2);  // Scale to 0-255
                g = (g << 2) | (g >> 4);  // Scale to 0-255
                b = (b << 3) | (b >> 2);  // Scale to 0-255

                // Apply color multiplication
                r = (r * colorFactor) >> 8;
                g = (g * colorFactor) >> 8;
                b = (b * colorFactor) >> 8;

                // Scale back to RGB565
                r = r >> 3;
                g = g >> 2;
                b = b >> 3;

                srcColor = (r << 11) | (g << 5) | b;
                alpha = (alpha * coloring.color.data[0]) >> 8;
            }

            uint8_t invAlpha = 255 - alpha;

            // Extract source and destination components with proper scaling
            uint32_t src_r = ((srcColor >> 11) & 0x1F) << 3;  // Scale to 0-255
            uint32_t src_g = ((srcColor >> 5) & 0x3F) << 2;   // Scale to 0-255
            uint32_t src_b = (srcColor & 0x1F) << 3;          // Scale to 0-255

            uint32_t dst_r = ((*dstPixel >> 11) & 0x1F) << 3;
            uint32_t dst_g = ((*dstPixel >> 5) & 0x3F) << 2;
            uint32_t dst_b = (*dstPixel & 0x1F) << 3;

            // Blend with proper scaling
            uint32_t final_r = (src_r * alpha + dst_r * invAlpha) >> 8;
            uint32_t final_g = (src_g * alpha + dst_g * invAlpha) >> 8;
            uint32_t final_b = (src_b * alpha + dst_b * invAlpha) >> 8;

            // Convert back to RGB565 with proper rounding
            final_r = (final_r + 4) >> 3;  // Add rounding factor
            final_g = (final_g + 2) >> 2;
            final_b = (final_b + 4) >> 3;

            // Clamp values
            final_r = (final_r > 0x1F) ? 0x1F : final_r;
            final_g = (final_g > 0x3F) ? 0x3F : final_g;
            final_b = (final_b > 0x1F) ? 0x1F : final_b;

            // Pack back into RGB565
            *dstPixel = (final_r << 11) | (final_g << 5) | final_b;
        }
    }

void BlendFunctions::BlendSolidRowRGB24(uint8_t *dstRow,
                                        const uint8_t *srcRow,
                                        size_t rowLength,
                                        const PixelFormatInfo &targetInfo,
                                        const PixelFormatInfo &sourceInfo,
                                        Coloring coloring,
                                        bool useSolidColor,
                                        BlendContext& context)
{
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, 1);
    convertColorToRGB24(coloring.color.data, colorDataAsRGB, 1);

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;

    uint8_t alpha = 255; // Default alpha
    if (context.mode != BlendMode::COLORINGONLY)
    {
        alpha = (*reinterpret_cast<const uint32_t *>(srcRow) >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
    }

    if (alpha == 0)
    {
        return;
    }

    if (colorFactor != 0)
    {
        srcRGB24[0] = (srcRGB24[0] * colorDataAsRGB[0]) >> 8;
        srcRGB24[1] = (srcRGB24[1] * colorDataAsRGB[1]) >> 8;
        srcRGB24[2] = (srcRGB24[2] * colorDataAsRGB[2]) >> 8;
        alpha = (alpha * colorDataAsRGB[2]) >> 8;
    }

    uint8_t invAlpha = 255 - alpha;

    for (size_t i = 0; i < rowLength * 3; i += 3)
    {
        uint8_t srcFactorR, dstFactorR;
        uint8_t srcFactorG, dstFactorG;
        uint8_t srcFactorB, dstFactorB;

        switch (context.colorBlendFactorSrc)
        {
        case BlendFactor::Zero:
            srcFactorR = srcFactorG = srcFactorB = 0;
            break;
        case BlendFactor::One:
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        case BlendFactor::SourceAlpha:
            srcFactorR = srcFactorG = srcFactorB = alpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            srcFactorR = srcFactorG = srcFactorB = 255 - alpha;
            break;
        case BlendFactor::DestAlpha:
            srcFactorR = 255;
            srcFactorG = 255;
            srcFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = 0;
            srcFactorG = 0;
            srcFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            srcFactorR = srcRGB24[0];
            srcFactorG = srcRGB24[1];
            srcFactorB = srcRGB24[2];
            break;
        case BlendFactor::DestColor:
            srcFactorR = dstRow[i];
            srcFactorG = dstRow[i + 1];
            srcFactorB = dstRow[i + 2];
            break;
        case BlendFactor::InverseSourceColor:
            srcFactorR = 255 - srcRGB24[0];
            srcFactorG = 255 - srcRGB24[1];
            srcFactorB = 255 - srcRGB24[2];
            break;
        case BlendFactor::InverseDestColor:
            srcFactorR = 255 - dstRow[i];
            srcFactorG = 255 - dstRow[i + 1];
            srcFactorB = 255 - dstRow[i + 2];
            break;
        default:
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        }

        switch (context.colorBlendFactorDst)
        {
        case BlendFactor::Zero:
            dstFactorR = dstFactorG = dstFactorB = 0;
            break;
        case BlendFactor::One:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        case BlendFactor::SourceAlpha:
            dstFactorR = dstFactorG = dstFactorB = alpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            dstFactorR = dstFactorG = dstFactorB = 255 - alpha;
            break;
        case BlendFactor::DestAlpha:
            dstFactorR = 255;
            dstFactorG = 255;
            dstFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = 0;
            dstFactorG = 0;
            dstFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            dstFactorR = srcRGB24[0];
            dstFactorG = srcRGB24[1];
            dstFactorB = srcRGB24[2];
            break;
        case BlendFactor::DestColor:
            dstFactorR = dstRow[i];
            dstFactorG = dstRow[i + 1];
            dstFactorB = dstRow[i + 2];
            break;
        case BlendFactor::InverseSourceColor:
            dstFactorR = 255 - srcRGB24[0];
            dstFactorG = 255 - srcRGB24[1];
            dstFactorB = 255 - srcRGB24[2];
            break;
        case BlendFactor::InverseDestColor:
            dstFactorR = 255 - dstRow[i];
            dstFactorG = 255 - dstRow[i + 1];
            dstFactorB = 255 - dstRow[i + 2];
            break;
        default:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        }

        switch (context.colorBlendOperation)
        {
            case BlendOperation::Add:
                dstRow[i] = (srcRGB24[0] * srcFactorR + dstRow[i] * dstFactorR) >> 8;
                dstRow[i + 1] = (srcRGB24[1] * srcFactorG + dstRow[i + 1] * dstFactorG) >> 8;
                dstRow[i + 2] = (srcRGB24[2] * srcFactorB + dstRow[i + 2] * dstFactorB) >> 8;
                break;
            case BlendOperation::Subtract:
            {
                    int tempR = (srcRGB24[0] * srcFactorR - dstRow[i] * dstFactorR) >> 8;
                    int tempG = (srcRGB24[1] * srcFactorG - dstRow[i + 1] * dstFactorG) >> 8;
                    int tempB = (srcRGB24[2] * srcFactorB - dstRow[i + 2] * dstFactorB) >> 8;

                    dstRow[i] = tempR < 0 ? 0 : (tempR > 255 ? 255 : tempR);
                    dstRow[i + 1] = tempG < 0 ? 0 : (tempG > 255 ? 255 : tempG);
                    dstRow[i + 2] = tempB < 0 ? 0 : (tempB > 255 ? 255 : tempB);
                    break;
            }
            case BlendOperation::ReverseSubtract:
            {
                int tempR = (dstRow[i] * dstFactorR - srcRGB24[0] * srcFactorR) >> 8;
                int tempG = (dstRow[i + 1] * dstFactorG - srcRGB24[1] * srcFactorG) >> 8;
                int tempB = (dstRow[i + 2] * dstFactorB - srcRGB24[2] * srcFactorB) >> 8;

                dstRow[i] = tempR < 0 ? 0 : (tempR > 255 ? 255 : tempR);
                dstRow[i + 1] = tempG < 0 ? 0 : (tempG > 255 ? 255 : tempG);
                dstRow[i + 2] = tempB < 0 ? 0 : (tempB > 255 ? 255 : tempB);
                break;
            }

            default:
                break;
        }
    }
}

void BlendFunctions::BlendRGB24(uint8_t *dstRow,
                                const uint8_t *srcRow,
                                size_t rowLength,
                                const PixelFormatInfo &targetInfo,
                                const PixelFormatInfo &sourceInfo,
                                Coloring coloring,
                                bool useSolidColor,
                                BlendContext& context)
{
    // Conversion function for the source format could be either rgb24 or bgr24
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);
    convertColorToRGB24(coloring.color.data, colorDataAsRGB, 1);

    const uint8_t *srcPixel = srcRow;
    uint8_t *dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;
    for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, dstPixel += targetInfo.bytesPerPixel)
    {
        uint8_t alpha = 255;

        uint8_t *srcColor = &srcRGB24[i * 3];
        if (sourceInfo.format == PixelFormat::GRAYSCALE8)
        {
            uint8_t grayValue = srcPixel[0];
            alpha = (grayValue == 0) ? 0 : 255;
        }
        else if (context.mode == BlendMode::COLORINGONLY)
        {
            alpha = 255;
        }
        else
        {
            alpha = (*reinterpret_cast<const uint32_t *>(srcPixel) >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
        }

        if (alpha == 0)
        {
            continue;
        }

        if (colorFactor != 0)
        {
            srcColor[0] = (srcColor[0] * colorDataAsRGB[0]) >> 8;
            srcColor[1] = (srcColor[1] * colorDataAsRGB[1]) >> 8;
            srcColor[2] = (srcColor[2] * colorDataAsRGB[2]) >> 8;
            alpha = (alpha * coloring.color.data[0]) >> 8;
        }

        if (alpha == 255)
        {
            dstPixel[0] = srcColor[0];
            dstPixel[1] = srcColor[1];
            dstPixel[2] = srcColor[2];
            continue;
        }

        uint8_t invAlpha = 255 - alpha;

        uint8_t srcFactorR, dstFactorR;
        uint8_t srcFactorG, dstFactorG;
        uint8_t srcFactorB, dstFactorB;

        switch (context.colorBlendFactorSrc)
        {
        case BlendFactor::Zero:
            srcFactorR = srcFactorG = srcFactorB = 0;
            break;
        case BlendFactor::One:
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        case BlendFactor::SourceAlpha:
            srcFactorR = srcFactorG = srcFactorB = alpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            srcFactorR = srcFactorG = srcFactorB = 255 - alpha;
            break;
        case BlendFactor::DestAlpha:
            srcFactorR = 255;
            srcFactorG = 255;
            srcFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = 0;
            srcFactorG = 0;
            srcFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            srcFactorR = srcColor[0];
            srcFactorG = srcColor[1];
            srcFactorB = srcColor[2];
            break;
        case BlendFactor::DestColor:
            srcFactorR = dstPixel[0];
            srcFactorG = dstPixel[1];
            srcFactorB = dstPixel[2];
            break;
        case BlendFactor::InverseSourceColor:
            srcFactorR = 255 - srcColor[0];
            srcFactorG = 255 - srcColor[1];
            srcFactorB = 255 - srcColor[2];
            break;
        case BlendFactor::InverseDestColor:
            srcFactorR = 255 - dstPixel[0];
            srcFactorG = 255 - dstPixel[1];
            srcFactorB = 255 - dstPixel[2];
            break;
        default:
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        }

        switch (context.colorBlendFactorDst)
        {
        case BlendFactor::Zero:
            dstFactorR = dstFactorG = dstFactorB = 0;
            break;
        case BlendFactor::One:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        case BlendFactor::SourceAlpha:
            dstFactorR = dstFactorG = dstFactorB = alpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            dstFactorR = dstFactorG = dstFactorB = 255 - alpha;
            break;
        case BlendFactor::DestAlpha:
            dstFactorR = 255;
            dstFactorG = 255;
            dstFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = 0;
            dstFactorG = 0;
            dstFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            dstFactorR = srcColor[0];
            dstFactorG = srcColor[1];
            dstFactorB = srcColor[2];
            break;
        case BlendFactor::DestColor:
            dstFactorR = dstPixel[0];
            dstFactorG = dstPixel[1];
            dstFactorB = dstPixel[2];
            break;
        case BlendFactor::InverseSourceColor:
            dstFactorR = 255 - srcColor[0];
            dstFactorG = 255 - srcColor[1];
            dstFactorB = 255 - srcColor[2];
            break;
        case BlendFactor::InverseDestColor:
            dstFactorR = 255 - dstPixel[0];
            dstFactorG = 255 - dstPixel[1];
            dstFactorB = 255 - dstPixel[2];
            break;
        default:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        }

        switch (context.colorBlendOperation)
        {
            case BlendOperation::Add:
                dstPixel[0] = (srcColor[0] * srcFactorR + dstPixel[0] * dstFactorR) >> 8;
                dstPixel[1] = (srcColor[1] * srcFactorG + dstPixel[1] * dstFactorG) >> 8;
                dstPixel[2] = (srcColor[2] * srcFactorB + dstPixel[2] * dstFactorB) >> 8;
                break;
            case BlendOperation::Subtract:
                dstPixel[0] = ((srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8) < 0 ? 0 : (((srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8) > 255 ? 255 : (srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8);
                dstPixel[1] = ((srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8) < 0 ? 0 : (((srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8) > 255 ? 255 : (srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8);
                dstPixel[2] = ((srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8) < 0 ? 0 : (((srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8) > 255 ? 255 : (srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8);
                break;
            case BlendOperation::ReverseSubtract:
                dstPixel[0] = ((dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8) < 0 ? 0 : (((dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8) > 255 ? 255 : (dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8);
                dstPixel[1] = ((dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8) < 0 ? 0 : (((dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8) > 255 ? 255 : (dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8);
                dstPixel[2] = ((dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8) < 0 ? 0 : (((dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8) > 255 ? 255 : (dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8);
                break;
            default:
                break;
        }
    }
}

void BlendFunctions::BlendRGBA32ToRGB24(uint8_t *dstRow,
                                        const uint8_t *srcRow,
                                        size_t rowLength,
                                        const PixelFormatInfo &targetInfo,
                                        const PixelFormatInfo &sourceInfo,
                                        Coloring coloring,
                                        bool useSolidColor,
                                        BlendContext& context)
{
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);
    convertColorToRGB24(coloring.color.data, colorDataAsRGB, 1);


    convertToRGB24(srcRow, srcRGB24, rowLength);

    const uint8_t *srcPixel = srcRow;
    uint8_t *dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled * coloring.color.data[0];
    uint8_t inverseColorFactor = 255 - colorFactor;

    for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, dstPixel += targetInfo.bytesPerPixel)
    {
        uint8_t alpha = (context.mode == BlendMode::COLORINGONLY) * 255 + (context.mode != BlendMode::COLORINGONLY) * srcPixel[3];

        uint8_t mask = -(alpha != 0);
        alpha &= mask;

        uint8_t *srcColor = &srcRGB24[i * 3];

        if(colorFactor)
        {
            srcColor[0] = (srcColor[0] * colorDataAsRGB[0]) >> 8;
            srcColor[1] = (srcColor[1] * colorDataAsRGB[1]) >> 8;
            srcColor[2] = (srcColor[2] * colorDataAsRGB[2]) >> 8;
            alpha = (alpha * coloring.color.data[0]) >> 8;
        }
        uint8_t invAlpha = 255 - alpha;

        uint8_t srcFactorR, dstFactorR;
        uint8_t srcFactorG, dstFactorG;
        uint8_t srcFactorB, dstFactorB;

        switch (context.colorBlendFactorSrc)
        {
        case BlendFactor::Zero:
            srcFactorR = srcFactorG = srcFactorB = 0;
            break;
        case BlendFactor::One:
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        case BlendFactor::SourceAlpha:
            srcFactorR = srcFactorG = srcFactorB = alpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            srcFactorR = srcFactorG = srcFactorB = 255 - alpha;
            break;
        case BlendFactor::DestAlpha:
            srcFactorR = 255;
            srcFactorG = 255;
            srcFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = 0;
            srcFactorG = 0;
            srcFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            srcFactorR = srcColor[0];
            srcFactorG = srcColor[1];
            srcFactorB = srcColor[2];
            break;
        case BlendFactor::DestColor:
            srcFactorR = dstPixel[0];
            srcFactorG = dstPixel[1];
            srcFactorB = dstPixel[2];
            break;
        case BlendFactor::InverseSourceColor:
            srcFactorR = 255 - srcColor[0];
            srcFactorG = 255 - srcColor[1];
            srcFactorB = 255 - srcColor[2];
            break;
        case BlendFactor::InverseDestColor:
            srcFactorR = 255 - dstPixel[0];
            srcFactorG = 255 - dstPixel[1];
            srcFactorB = 255 - dstPixel[2];
            break;
        default:
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        }

        switch (context.colorBlendFactorDst)
        {
        case BlendFactor::Zero:
            dstFactorR = dstFactorG = dstFactorB = 0;
            break;
        case BlendFactor::One:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        case BlendFactor::SourceAlpha:
            dstFactorR = dstFactorG = dstFactorB = alpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            dstFactorR = dstFactorG = dstFactorB = 255 - alpha;
            break;
        case BlendFactor::DestAlpha:
            dstFactorR = 255;
            dstFactorG = 255;
            dstFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = 0;
            dstFactorG = 0;
            dstFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            dstFactorR = srcColor[0];
            dstFactorG = srcColor[1];
            dstFactorB = srcColor[2];
            break;
        case BlendFactor::DestColor:
            dstFactorR = dstPixel[0];
            dstFactorG = dstPixel[1];
            dstFactorB = dstPixel[2];
            break;
        case BlendFactor::InverseSourceColor:
            dstFactorR = 255 - srcColor[0];
            dstFactorG = 255 - srcColor[1];
            dstFactorB = 255 - srcColor[2];
            break;
        case BlendFactor::InverseDestColor:
            dstFactorR = 255 - dstPixel[0];
            dstFactorG = 255 - dstPixel[1];
            dstFactorB = 255 - dstPixel[2];
            break;
        default:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        }

        switch (context.colorBlendOperation)
        {
            case BlendOperation::Add:
                dstPixel[0] = (srcColor[0] * srcFactorR + dstPixel[0] * dstFactorR) >> 8;
                dstPixel[1] = (srcColor[1] * srcFactorG + dstPixel[1] * dstFactorG) >> 8;
                dstPixel[2] = (srcColor[2] * srcFactorB + dstPixel[2] * dstFactorB) >> 8;
                break;
            case BlendOperation::Subtract:
                dstPixel[0] = ((srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8) < 0 ? 0 : (((srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8) > 255 ? 255 : (srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8);
                dstPixel[1] = ((srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8) < 0 ? 0 : (((srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8) > 255 ? 255 : (srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8);
                dstPixel[2] = ((srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8) < 0 ? 0 : (((srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8) > 255 ? 255 : (srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8);
            break;
            case BlendOperation::ReverseSubtract:
                dstPixel[0] = ((dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8) < 0 ? 0 : (((dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8) > 255 ? 255 : (dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8);
                dstPixel[1] = ((dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8) < 0 ? 0 : (((dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8) > 255 ? 255 : (dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8);
                dstPixel[2] = ((dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8) < 0 ? 0 : (((dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8) > 255 ? 255 : (dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8);
                break;
            case BlendOperation::BitwiseAnd:
                dstPixel[0] = (srcColor[0] * srcFactorR & dstPixel[0] * dstFactorR) >> 8;
                dstPixel[1] = (srcColor[1] * srcFactorG & dstPixel[1] * dstFactorG) >> 8;
                dstPixel[2] = (srcColor[2] * srcFactorB & dstPixel[2] * dstFactorB) >> 8;
                break;
            default:
                break;
        }
    }
}