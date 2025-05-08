#include "../../BlendMode.h"

#include "../../BlendFunctions.h"

#include "../../../PixelFormat/PixelConverter.h"

#include "../../../PixelFormat/PixelFormatInfo.h"

#include <arm_neon.h>

using namespace Tergos2D;

void BlendFunctions::BlendSolidRowRGB24(uint8_t * dstRow,
    const uint8_t * srcRow,
        size_t rowLength,
        const PixelFormatInfo & targetInfo,
            const PixelFormatInfo & sourceInfo,
                Coloring coloring,
                bool useSolidColor,
                BlendContext & context) {
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
    if (context.mode != BlendMode::COLORINGONLY) {
        alpha = ( * reinterpret_cast <
            const uint32_t * > (srcRow) >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
    }

    if (alpha == 0) {
        return;
    }

    // Create NEON vectors for the source color
    uint8x8x3_t src_neon;
    src_neon.val[0] = vdup_n_u8(srcRGB24[0]);
    src_neon.val[1] = vdup_n_u8(srcRGB24[1]);
    src_neon.val[2] = vdup_n_u8(srcRGB24[2]);

    // Apply color tinting if needed
    if (colorFactor != 0) {
        uint8x8_t color_r = vdup_n_u8(colorDataAsRGB[0]);
        uint8x8_t color_g = vdup_n_u8(colorDataAsRGB[1]);
        uint8x8_t color_b = vdup_n_u8(colorDataAsRGB[2]);

        src_neon.val[0] = vshrn_n_u16(vmull_u8(src_neon.val[0], color_r), 8);
        src_neon.val[1] = vshrn_n_u16(vmull_u8(src_neon.val[1], color_g), 8);
        src_neon.val[2] = vshrn_n_u16(vmull_u8(src_neon.val[2], color_b), 8);
        alpha = (alpha * colorDataAsRGB[2]) >> 8;
    }

    // Create NEON vectors for blend factors
    uint8x8_t alpha_vec = vdup_n_u8(alpha);
    uint8x8_t inv_alpha_vec = vdup_n_u8(255 - alpha);
    uint8x8_t full_vec = vdup_n_u8(255);
    uint8x8_t zero_vec = vdup_n_u8(0);

    // Process 8 pixels at a time
    size_t vectorized_length = (rowLength / 8) * 8;
    size_t i = 0;
    for (i = 0; i < vectorized_length * 3; i += 24) // 8 pixels * 3 bytes
    {
        // Load 8 pixels of destination data
        uint8x8x3_t dst_neon = vld3_u8( & dstRow[i]);
        uint8x8x3_t src_factor, dst_factor;

        // Calculate source blend factors
        switch (context.colorBlendFactorSrc) {
        case BlendFactor::Zero:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = zero_vec;
            break;
        case BlendFactor::One:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = full_vec;
            break;
        case BlendFactor::SourceAlpha:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = alpha_vec;
            break;
        case BlendFactor::InverseSourceAlpha:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = inv_alpha_vec;
            break;
        case BlendFactor::DestAlpha:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = full_vec;
            break;
        case BlendFactor::InverseDestAlpha:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = zero_vec;
            break;
        case BlendFactor::SourceColor:
            src_factor.val[0] = src_neon.val[0];
            src_factor.val[1] = src_neon.val[1];
            src_factor.val[2] = src_neon.val[2];
            break;
        case BlendFactor::DestColor:
            src_factor.val[0] = dst_neon.val[0];
            src_factor.val[1] = dst_neon.val[1];
            src_factor.val[2] = dst_neon.val[2];
            break;
        case BlendFactor::InverseSourceColor:
            src_factor.val[0] = vsub_u8(full_vec, src_neon.val[0]);
            src_factor.val[1] = vsub_u8(full_vec, src_neon.val[1]);
            src_factor.val[2] = vsub_u8(full_vec, src_neon.val[2]);
            break;
        case BlendFactor::InverseDestColor:
            src_factor.val[0] = vsub_u8(full_vec, dst_neon.val[0]);
            src_factor.val[1] = vsub_u8(full_vec, dst_neon.val[1]);
            src_factor.val[2] = vsub_u8(full_vec, dst_neon.val[2]);
            break;
        default:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = full_vec;
            break;
        }

        // Calculate destination blend factors
        switch (context.colorBlendFactorDst) {
        case BlendFactor::Zero:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = zero_vec;
            break;
        case BlendFactor::One:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = full_vec;
            break;
        case BlendFactor::SourceAlpha:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = alpha_vec;
            break;
        case BlendFactor::InverseSourceAlpha:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = inv_alpha_vec;
            break;
        case BlendFactor::DestAlpha:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = full_vec;
            break;
        case BlendFactor::InverseDestAlpha:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = zero_vec;
            break;
        case BlendFactor::SourceColor:
            dst_factor.val[0] = src_neon.val[0];
            dst_factor.val[1] = src_neon.val[1];
            dst_factor.val[2] = src_neon.val[2];
            break;
        case BlendFactor::DestColor:
            dst_factor.val[0] = dst_neon.val[0];
            dst_factor.val[1] = dst_neon.val[1];
            dst_factor.val[2] = dst_neon.val[2];
            break;
        case BlendFactor::InverseSourceColor:
            dst_factor.val[0] = vsub_u8(full_vec, src_neon.val[0]);
            dst_factor.val[1] = vsub_u8(full_vec, src_neon.val[1]);
            dst_factor.val[2] = vsub_u8(full_vec, src_neon.val[2]);
            break;
        case BlendFactor::InverseDestColor:
            dst_factor.val[0] = vsub_u8(full_vec, dst_neon.val[0]);
            dst_factor.val[1] = vsub_u8(full_vec, dst_neon.val[1]);
            dst_factor.val[2] = vsub_u8(full_vec, dst_neon.val[2]);
            break;
        default:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = full_vec;
            break;
        }

        uint8x8x3_t result_neon;

        // Perform blend operation
        switch (context.colorBlendOperation) {
        case BlendOperation::Add: {
            // Multiply and add with proper scaling
            uint16x8_t temp_r = vmull_u8(src_neon.val[0], src_factor.val[0]);
            uint16x8_t temp_g = vmull_u8(src_neon.val[1], src_factor.val[1]);
            uint16x8_t temp_b = vmull_u8(src_neon.val[2], src_factor.val[2]);

            uint16x8_t dst_r = vmull_u8(dst_neon.val[0], dst_factor.val[0]);
            uint16x8_t dst_g = vmull_u8(dst_neon.val[1], dst_factor.val[1]);
            uint16x8_t dst_b = vmull_u8(dst_neon.val[2], dst_factor.val[2]);

            temp_r = vaddq_u16(temp_r, dst_r);
            temp_g = vaddq_u16(temp_g, dst_g);
            temp_b = vaddq_u16(temp_b, dst_b);

            result_neon.val[0] = vshrn_n_u16(temp_r, 8);
            result_neon.val[1] = vshrn_n_u16(temp_g, 8);
            result_neon.val[2] = vshrn_n_u16(temp_b, 8);
            break;
        }
        case BlendOperation::Subtract: {
            uint8x8_t resultR = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_neon.val[0], src_factor.val[0]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_neon.val[0], dst_factor.val[0]), 8))
            ));

            uint8x8_t resultG = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_neon.val[1], src_factor.val[1]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_neon.val[1], dst_factor.val[1]), 8))
            ));

            uint8x8_t resultB = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_neon.val[2], src_factor.val[2]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_neon.val[2], dst_factor.val[2]), 8))
            ));

            result_neon.val[0] = resultR;
            result_neon.val[1] = resultG;
            result_neon.val[2] = resultB;
            vst3_u8(&dstRow[i], result_neon);
            break;
        }
        case BlendOperation::ReverseSubtract: {
            uint8x8_t resultR = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_neon.val[0], dst_factor.val[0]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_neon.val[0], src_factor.val[0]), 8))
            ));

            uint8x8_t resultG = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_neon.val[1], dst_factor.val[1]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_neon.val[1], src_factor.val[1]), 8))
            ));

            uint8x8_t resultB = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_neon.val[2], dst_factor.val[2]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_neon.val[2], src_factor.val[2]), 8))
            ));

            result_neon.val[0] = resultR;
            result_neon.val[1] = resultG;
            result_neon.val[2] = resultB;
            vst3_u8(&dstRow[i], result_neon);
            break;
        }
        default:
            result_neon = dst_neon;
            break;
        }

        // Store the result back to memory
        vst3_u8( & dstRow[i], result_neon);
    }
    // Handle remaining pixels using scalar code
    for (; i < rowLength * 3; i += 3) {
        uint8_t srcFactorR, dstFactorR;
        uint8_t srcFactorG, dstFactorG;
        uint8_t srcFactorB, dstFactorB;

        // Calculate source blend factors
        switch (context.colorBlendFactorSrc) {
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
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = srcFactorG = srcFactorB = 0;
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

        // Calculate destination blend factors
        switch (context.colorBlendFactorDst) {
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
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = dstFactorG = dstFactorB = 0;
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

        // Perform blend operation
        switch (context.colorBlendOperation) {
        case BlendOperation::Add:
            dstRow[i] = (srcRGB24[0] * srcFactorR + dstRow[i] * dstFactorR) >> 8;
            dstRow[i + 1] = (srcRGB24[1] * srcFactorG + dstRow[i + 1] * dstFactorG) >> 8;
            dstRow[i + 2] = (srcRGB24[2] * srcFactorB + dstRow[i + 2] * dstFactorB) >> 8;
            break;
        case BlendOperation::Subtract: {
            int srcRedComponent = srcRGB24[0] * srcFactorR;
            int srcGreenComponent = srcRGB24[1] * srcFactorG;
            int srcBlueComponent = srcRGB24[2] * srcFactorB;

            int dstRedComponent = dstRow[i] * dstFactorR;
            int dstGreenComponent = dstRow[i + 1] * dstFactorG;
            int dstBlueComponent = dstRow[i + 2] * dstFactorB;

            int tempR = (srcRedComponent - dstRedComponent) >> 8;
            int tempG = (srcGreenComponent - dstGreenComponent) >> 8;
            int tempB = (srcBlueComponent - dstBlueComponent) >> 8;

            int finalR = tempR < 0 ? 0 : (tempR > 255 ? 255 : tempR);
            int finalG = tempG < 0 ? 0 : (tempG > 255 ? 255 : tempG);
            int finalB = tempB < 0 ? 0 : (tempB > 255 ? 255 : tempB);

            dstRow[i] = finalR;
            dstRow[i + 1] = finalG;
            dstRow[i + 2] = finalB;
            break;
        }
        case BlendOperation::ReverseSubtract: {
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

void BlendFunctions::BlendToRGB24Simple(uint8_t * dstRow,
    const uint8_t * srcRow,
        size_t rowLength,
        const PixelFormatInfo & targetInfo,
            const PixelFormatInfo & sourceInfo,
                Coloring coloring,
                bool useSolidColor,
                BlendContext & context) {
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);

    uint8_t colorFactor = coloring.colorEnabled * coloring.color.data[0];
    uint8_t inverseColorFactor = 255 - colorFactor;

    uint8x8x3_t color_neon;
    if (coloring.colorEnabled) {
        color_neon.val[0] = vdup_n_u8(colorDataAsRGB[0]);
        color_neon.val[1] = vdup_n_u8(colorDataAsRGB[1]);
        color_neon.val[2] = vdup_n_u8(colorDataAsRGB[2]);
    } 

    size_t vectorized_length = (rowLength / 8) * 8;
    size_t i;

    for (i = 0; i < vectorized_length; i += 8) {
        // Load 8 gray values
        uint8x8_t gray_values = vld1_u8( & srcRow[i * sourceInfo.bytesPerPixel]);

        // Create alpha mask based on gray values
        uint8x8_t alpha = vceq_u8(gray_values, vdup_n_u8(0));
        alpha = vmvn_u8(alpha); // Invert the mask

        // Early skip if all pixels are transparent
        if (vget_lane_u64(vreinterpret_u64_u8(alpha), 0) == 0) {
            continue;
        }

        uint8x8x3_t src_neon = vld3_u8( & srcRGB24[i * 3]);

        if (coloring.colorEnabled) {
            // Multiply RGB components with color
            uint16x8_t temp_r = vmull_u8(src_neon.val[0], color_neon.val[0]);
            uint16x8_t temp_g = vmull_u8(src_neon.val[1], color_neon.val[1]);
            uint16x8_t temp_b = vmull_u8(src_neon.val[2], color_neon.val[2]);

            src_neon.val[0] = vshrn_n_u16(temp_r, 8);
            src_neon.val[1] = vshrn_n_u16(temp_g, 8);
            src_neon.val[2] = vshrn_n_u16(temp_b, 8);

            // Adjust alpha
            uint16x8_t temp_alpha = vmull_u8(alpha, vdup_n_u8(coloring.color.data[0]));
            alpha = vshrn_n_u16(temp_alpha, 8);
        }

        // Calculate inverse alpha
        uint8x8_t inv_alpha = vsub_u8(vdup_n_u8(255), alpha);

        // Load destination pixels
        uint8x8x3_t dst_neon = vld3_u8( & dstRow[i * 3]);

        // Blend calculations
        uint16x8_t blend_r = vmull_u8(src_neon.val[0], alpha);
        uint16x8_t blend_g = vmull_u8(src_neon.val[1], alpha);
        uint16x8_t blend_b = vmull_u8(src_neon.val[2], alpha);

        blend_r = vmlal_u8(blend_r, dst_neon.val[0], inv_alpha);
        blend_g = vmlal_u8(blend_g, dst_neon.val[1], inv_alpha);
        blend_b = vmlal_u8(blend_b, dst_neon.val[2], inv_alpha);

        // Shift right by 8 and store results
        uint8x8x3_t result_neon;
        result_neon.val[0] = vshrn_n_u16(blend_r, 8);
        result_neon.val[1] = vshrn_n_u16(blend_g, 8);
        result_neon.val[2] = vshrn_n_u16(blend_b, 8);

        // Store the result
        vst3_u8( & dstRow[i * 3], result_neon);
    }

    // Handle remaining pixels
    for (; i < rowLength; ++i) {
        const uint8_t * srcPixel = & srcRow[i * sourceInfo.bytesPerPixel];
        uint8_t * dstPixel = & dstRow[i * 3];
        uint8_t * srcColor = & srcRGB24[i * 3];

        uint8_t grayValue = srcPixel[0];
        uint8_t alpha = (grayValue == 0) ? 0 : 255;

        if (alpha == 0) {
            continue;
        }

        // Apply coloring if needed
        if (coloring.colorEnabled) {
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

void BlendFunctions::BlendRGB24(uint8_t* dstRow,
    const uint8_t* srcRow,
    size_t rowLength,
    const PixelFormatInfo& targetInfo,
    const PixelFormatInfo& sourceInfo,
    Coloring coloring,
    bool useSolidColor,
    BlendContext& context) {

    // Conversion function for the source format could be either rgb24 or bgr24
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);
    convertColorToRGB24(coloring.color.data, colorDataAsRGB, 1);

    const uint8_t* srcPixel = srcRow;
    uint8_t* dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;

    // Create NEON vectors for constant values
    uint8x8_t v_255 = vdup_n_u8(255);
    uint8x8_t v_colorFactor = vdup_n_u8(colorFactor);
    uint8x8_t v_colorDataR = vdup_n_u8(colorDataAsRGB[0]);
    uint8x8_t v_colorDataG = vdup_n_u8(colorDataAsRGB[1]);
    uint8x8_t v_colorDataB = vdup_n_u8(colorDataAsRGB[2]);

    // Process 8 pixels at a time
    size_t vectorized_length = rowLength - (rowLength % 8);
    size_t i = 0;

    for (i = 0; i < vectorized_length; i += 8) {
        // Load 8 pixels worth of data
        uint8x8x3_t src_rgb = vld3_u8(&srcRGB24[i * 3]);
        uint8x8x3_t dst_rgb = vld3_u8(&dstPixel[i * targetInfo.bytesPerPixel]);

        // Handle alpha
        uint8x8_t alpha;
        if (sourceInfo.format == PixelFormat::GRAYSCALE8) {
            // For grayscale, create binary alpha (0 or 255)
            uint8x8_t gray = vld1_u8(&srcPixel[i]);
            alpha = vcgt_u8(gray, vdup_n_u8(0));
            alpha = vand_u8(alpha, v_255);
        } else if (context.mode == BlendMode::COLORINGONLY) {
            alpha = v_255;
        } else {
            // Handle alpha shift based on common alpha shift values
            uint8x8_t src_pixels = vld1_u8(&srcPixel[i * sourceInfo.bytesPerPixel]);
            uint32x2_t alpha_temp = vreinterpret_u32_u8(src_pixels);

            // Handle different alpha shift values using compile-time constants
            switch(sourceInfo.alphaShift) {
                case 24:
                    alpha_temp = vshr_n_u32(alpha_temp, 24);
                    break;
                case 16:
                    alpha_temp = vshr_n_u32(alpha_temp, 16);
                    break;
                case 8:
                    alpha_temp = vshr_n_u32(alpha_temp, 8);
                    break;
                case 0:
                    // No shift needed
                    break;
                default:
                    // Fall back to scalar processing for unusual alpha shifts
                    uint8_t scalar_alpha[8];
                    for(int j = 0; j < 8; j++) {
                        scalar_alpha[j] = (srcPixel[i * sourceInfo.bytesPerPixel + j] >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
                    }
                    alpha = vld1_u8(scalar_alpha);
                    break;
            }

            if (sourceInfo.alphaShift <= 24) { // Only if we used the NEON shift
                alpha = vreinterpret_u8_u32(alpha_temp);
                alpha = vand_u8(alpha, vdup_n_u8(sourceInfo.alphaMask));
            }
        }

        // Apply color factor if needed
        if (colorFactor != 0) {
            // Multiply source colors with color data
            uint16x8_t temp_r = vmull_u8(src_rgb.val[0], v_colorDataR);
            uint16x8_t temp_g = vmull_u8(src_rgb.val[1], v_colorDataG);
            uint16x8_t temp_b = vmull_u8(src_rgb.val[2], v_colorDataB);

            src_rgb.val[0] = vshrn_n_u16(temp_r, 8);
            src_rgb.val[1] = vshrn_n_u16(temp_g, 8);
            src_rgb.val[2] = vshrn_n_u16(temp_b, 8);

            // Adjust alpha
            uint16x8_t temp_alpha = vmull_u8(alpha, v_colorFactor);
            alpha = vshrn_n_u16(temp_alpha, 8);
        }

        // Calculate inverse alpha
        uint8x8_t inv_alpha = vsub_u8(v_255, alpha);

        // Prepare blend factors based on context
        uint8x8_t src_factor_r, src_factor_g, src_factor_b;
        uint8x8_t dst_factor_r, dst_factor_g, dst_factor_b;

        // Set blend factors based on context.colorBlendFactorSrc
        switch (context.colorBlendFactorSrc) {
            case BlendFactor::Zero:
                src_factor_r = src_factor_g = src_factor_b = vdup_n_u8(0);
                break;
            case BlendFactor::One:
                src_factor_r = src_factor_g = src_factor_b = v_255;
                break;
            case BlendFactor::SourceAlpha:
                src_factor_r = src_factor_g = src_factor_b = alpha;
                break;
            case BlendFactor::InverseSourceAlpha:
                src_factor_r = src_factor_g = src_factor_b = inv_alpha;
                break;
            case BlendFactor::DestAlpha:
                src_factor_r = src_factor_g = src_factor_b = v_255;
                break;
            case BlendFactor::InverseDestAlpha:
                src_factor_r = src_factor_g = src_factor_b = vdup_n_u8(0);
                break;
            case BlendFactor::SourceColor:
                src_factor_r = src_rgb.val[0];
                src_factor_g = src_rgb.val[1];
                src_factor_b = src_rgb.val[2];
                break;
            case BlendFactor::DestColor:
                src_factor_r = dst_rgb.val[0];
                src_factor_g = dst_rgb.val[1];
                src_factor_b = dst_rgb.val[2];
                break;
            case BlendFactor::InverseSourceColor:
                src_factor_r = vsub_u8(v_255, src_rgb.val[0]);
                src_factor_g = vsub_u8(v_255, src_rgb.val[1]);
                src_factor_b = vsub_u8(v_255, src_rgb.val[2]);
                break;
            case BlendFactor::InverseDestColor:
                src_factor_r = vsub_u8(v_255, dst_rgb.val[0]);
                src_factor_g = vsub_u8(v_255, dst_rgb.val[1]);
                src_factor_b = vsub_u8(v_255, dst_rgb.val[2]);
                break;
            default:
                src_factor_r = src_factor_g = src_factor_b = v_255;
                break;
        }

        // Set blend factors based on context.colorBlendFactorDst
        switch (context.colorBlendFactorDst) {
            case BlendFactor::Zero:
                dst_factor_r = dst_factor_g = dst_factor_b = vdup_n_u8(0);
                break;
            case BlendFactor::One:
                dst_factor_r = dst_factor_g = dst_factor_b = v_255;
                break;
            case BlendFactor::SourceAlpha:
                dst_factor_r = dst_factor_g = dst_factor_b = alpha;
                break;
            case BlendFactor::InverseSourceAlpha:
                dst_factor_r = dst_factor_g = dst_factor_b = inv_alpha;
                break;
            case BlendFactor::DestAlpha:
                dst_factor_r = dst_factor_g = dst_factor_b = v_255;
                break;
            case BlendFactor::InverseDestAlpha:
                dst_factor_r = dst_factor_g = dst_factor_b = vdup_n_u8(0);
                break;
            case BlendFactor::SourceColor:
                dst_factor_r = src_rgb.val[0];
                dst_factor_g = src_rgb.val[1];
                dst_factor_b = src_rgb.val[2];
                break;
            case BlendFactor::DestColor:
                dst_factor_r = dst_rgb.val[0];
                dst_factor_g = dst_rgb.val[1];
                dst_factor_b = dst_rgb.val[2];
                break;
            case BlendFactor::InverseSourceColor:
                dst_factor_r = vsub_u8(v_255, src_rgb.val[0]);
                dst_factor_g = vsub_u8(v_255, src_rgb.val[1]);
                dst_factor_b = vsub_u8(v_255, src_rgb.val[2]);
                break;
            case BlendFactor::InverseDestColor:
                dst_factor_r = vsub_u8(v_255, dst_rgb.val[0]);
                dst_factor_g = vsub_u8(v_255, dst_rgb.val[1]);
                dst_factor_b = vsub_u8(v_255, dst_rgb.val[2]);
                break;
            default:
                dst_factor_r = dst_factor_g = dst_factor_b = v_255;
                break;
        }

        // Perform blending operation
        switch (context.colorBlendOperation) {
            case BlendOperation::Add: {
                uint16x8_t blend_r = vaddq_u16(vmull_u8(src_rgb.val[0], src_factor_r),
                                              vmull_u8(dst_rgb.val[0], dst_factor_r));
                uint16x8_t blend_g = vaddq_u16(vmull_u8(src_rgb.val[1], src_factor_g),
                                              vmull_u8(dst_rgb.val[1], dst_factor_g));
                uint16x8_t blend_b = vaddq_u16(vmull_u8(src_rgb.val[2], src_factor_b),
                                              vmull_u8(dst_rgb.val[2], dst_factor_b));

                dst_rgb.val[0] = vshrn_n_u16(blend_r, 8);
                dst_rgb.val[1] = vshrn_n_u16(blend_g, 8);
                dst_rgb.val[2] = vshrn_n_u16(blend_b, 8);
                break;
            }
            case BlendOperation::Subtract: {
                dst_rgb.val[0] = vqmovun_s16(vsubq_s16(
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[0], src_factor_r), 8)),
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[0], dst_factor_r), 8))
                ));

                dst_rgb.val[1] = vqmovun_s16(vsubq_s16(
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[1], src_factor_g), 8)),
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[1], dst_factor_g), 8))
                ));

                dst_rgb.val[2] = vqmovun_s16(vsubq_s16(
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[2], src_factor_b), 8)),
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[2], dst_factor_b), 8))
                ));
                break;
            }
            case BlendOperation::ReverseSubtract: {
                dst_rgb.val[0] = vqmovun_s16(vsubq_s16(
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[0], dst_factor_r), 8)),
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[0], src_factor_r), 8))
                ));

                dst_rgb.val[1] = vqmovun_s16(vsubq_s16(
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[1], dst_factor_g), 8)),
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[1], src_factor_g), 8))
                ));

                dst_rgb.val[2] = vqmovun_s16(vsubq_s16(
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[2], dst_factor_b), 8)),
                    vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[2], src_factor_b), 8))
                ));
                break;
            }
        }

        // Store results
        vst3_u8(&dstPixel[i * targetInfo.bytesPerPixel], dst_rgb);
    }

    for (; i < rowLength; ++i)
    {
        const uint8_t* srcPixel = &srcRow[i * sourceInfo.bytesPerPixel];
        uint8_t* dstPixel = &dstRow[i * targetInfo.bytesPerPixel];
        uint8_t* srcColor = &srcRGB24[i * 3];

        uint8_t alpha = 255;

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
            alpha = (*reinterpret_cast<const uint32_t*>(srcPixel) >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
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

void BlendFunctions::BlendRGBA32ToRGB24(uint8_t * dstRow,
    const uint8_t * srcRow,
        size_t rowLength,
        const PixelFormatInfo & targetInfo,
            const PixelFormatInfo & sourceInfo,
                Coloring coloring,
                bool useSolidColor,
                BlendContext & context) {
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);
    convertColorToRGB24(coloring.color.data, colorDataAsRGB, 1);

    uint8_t colorFactor = coloring.colorEnabled * coloring.color.data[0];
    uint8_t inverseColorFactor = 255 - colorFactor;

    const uint8x8_t vec_255 = vdup_n_u8(255);
    const uint8x8_t vec_0 = vdup_n_u8(0);

    uint8x8x3_t color_neon;
    if (colorFactor) {
        color_neon.val[0] = vdup_n_u8(colorDataAsRGB[0]);
        color_neon.val[1] = vdup_n_u8(colorDataAsRGB[1]);
        color_neon.val[2] = vdup_n_u8(colorDataAsRGB[2]);
    }

    size_t vectorized_length = (rowLength / 8) * 8;
    size_t i;

    for (i = 0; i < vectorized_length; i += 8) {
        uint8x8x4_t src_rgba = vld4_u8( & srcRow[i * 4]);
        uint8x8x3_t src_rgb = vld3_u8( & srcRGB24[i * 3]);
        uint8x8x3_t dst_rgb = vld3_u8( & dstRow[i * 3]);

        uint8x8_t alpha;
        if (context.mode == BlendMode::COLORINGONLY) {
            alpha = vec_255;
        } else {
            alpha = src_rgba.val[3];
        }

        uint8x8_t alpha_mask = vcgt_u8(alpha, vec_0);
        alpha = vand_u8(alpha, alpha_mask);

        if (colorFactor) {
            uint16x8_t temp_r = vmull_u8(src_rgb.val[0], color_neon.val[0]);
            uint16x8_t temp_g = vmull_u8(src_rgb.val[1], color_neon.val[1]);
            uint16x8_t temp_b = vmull_u8(src_rgb.val[2], color_neon.val[2]);

            src_rgb.val[0] = vshrn_n_u16(temp_r, 8);
            src_rgb.val[1] = vshrn_n_u16(temp_g, 8);
            src_rgb.val[2] = vshrn_n_u16(temp_b, 8);

            uint16x8_t temp_alpha = vmull_u8(alpha, vdup_n_u8(coloring.color.data[0]));
            alpha = vshrn_n_u16(temp_alpha, 8);
        }

        uint8x8x3_t src_factor, dst_factor;

        switch (context.colorBlendFactorSrc) {
        case BlendFactor::Zero:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = vec_0;
            break;
        case BlendFactor::One:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = vec_255;
            break;
        case BlendFactor::SourceAlpha:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = alpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = vsub_u8(vec_255, alpha);
            break;
        case BlendFactor::DestAlpha:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = vec_255;
            break;
        case BlendFactor::InverseDestAlpha:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = vec_0;
            break;
        case BlendFactor::SourceColor:
            src_factor = src_rgb;
            break;
        case BlendFactor::DestColor:
            src_factor = dst_rgb;
            break;
        case BlendFactor::InverseSourceColor:
            src_factor.val[0] = vsub_u8(vec_255, src_rgb.val[0]);
            src_factor.val[1] = vsub_u8(vec_255, src_rgb.val[1]);
            src_factor.val[2] = vsub_u8(vec_255, src_rgb.val[2]);
            break;
        case BlendFactor::InverseDestColor:
            src_factor.val[0] = vsub_u8(vec_255, dst_rgb.val[0]);
            src_factor.val[1] = vsub_u8(vec_255, dst_rgb.val[1]);
            src_factor.val[2] = vsub_u8(vec_255, dst_rgb.val[2]);
            break;
        default:
            src_factor.val[0] = src_factor.val[1] = src_factor.val[2] = vec_255;
            break;
        }

        switch (context.colorBlendFactorDst) {
        case BlendFactor::Zero:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = vec_0;
            break;
        case BlendFactor::One:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = vec_255;
            break;
        case BlendFactor::SourceAlpha:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = alpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = vsub_u8(vec_255, alpha);
            break;
        case BlendFactor::DestAlpha:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = vec_255;
            break;
        case BlendFactor::InverseDestAlpha:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = vec_0;
            break;
        case BlendFactor::SourceColor:
            dst_factor = src_rgb;
            break;
        case BlendFactor::DestColor:
            dst_factor = dst_rgb;
            break;
        case BlendFactor::InverseSourceColor:
            dst_factor.val[0] = vsub_u8(vec_255, src_rgb.val[0]);
            dst_factor.val[1] = vsub_u8(vec_255, src_rgb.val[1]);
            dst_factor.val[2] = vsub_u8(vec_255, src_rgb.val[2]);
            break;
        case BlendFactor::InverseDestColor:
            dst_factor.val[0] = vsub_u8(vec_255, dst_rgb.val[0]);
            dst_factor.val[1] = vsub_u8(vec_255, dst_rgb.val[1]);
            dst_factor.val[2] = vsub_u8(vec_255, dst_rgb.val[2]);
            break;
        default:
            dst_factor.val[0] = dst_factor.val[1] = dst_factor.val[2] = vec_255;
            break;
        }

        uint8x8x3_t result;

        switch (context.colorBlendOperation) {
        case BlendOperation::Add: {
            uint16x8_t temp_r = vmull_u8(src_rgb.val[0], src_factor.val[0]);
            uint16x8_t temp_g = vmull_u8(src_rgb.val[1], src_factor.val[1]);
            uint16x8_t temp_b = vmull_u8(src_rgb.val[2], src_factor.val[2]);

            uint16x8_t dst_r = vmull_u8(dst_rgb.val[0], dst_factor.val[0]);
            uint16x8_t dst_g = vmull_u8(dst_rgb.val[1], dst_factor.val[1]);
            uint16x8_t dst_b = vmull_u8(dst_rgb.val[2], dst_factor.val[2]);

            temp_r = vaddq_u16(temp_r, dst_r);
            temp_g = vaddq_u16(temp_g, dst_g);
            temp_b = vaddq_u16(temp_b, dst_b);

            result.val[0] = vshrn_n_u16(temp_r, 8);
            result.val[1] = vshrn_n_u16(temp_g, 8);
            result.val[2] = vshrn_n_u16(temp_b, 8);
            break;
        }
        case BlendOperation::Subtract: {
            result.val[0] = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[0], src_factor.val[0]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[0], dst_factor.val[0]), 8))
            ));

            result.val[1] = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[1], src_factor.val[1]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[1], dst_factor.val[1]), 8))
            ));

            result.val[2] = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[2], src_factor.val[2]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[2], dst_factor.val[2]), 8))
            ));
            break;
        }
        case BlendOperation::ReverseSubtract: {
            result.val[0] = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[0], dst_factor.val[0]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[0], src_factor.val[0]), 8))
            ));

            result.val[1] = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[1], dst_factor.val[1]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[1], src_factor.val[1]), 8))
            ));

            result.val[2] = vqmovun_s16(vsubq_s16(
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(dst_rgb.val[2], dst_factor.val[2]), 8)),
                vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(src_rgb.val[2], src_factor.val[2]), 8))
            ));
            break;
        }
        default:
            result = dst_rgb;
            break;
        }

        vst3_u8( & dstRow[i * 3], result);
    }

    // Handle remaining pixels
    for (; i < rowLength; ++i) {
        const uint8_t * srcPixel = & srcRow[i * sourceInfo.bytesPerPixel];
        uint8_t * dstPixel = & dstRow[i * 3];
        uint8_t * srcColor = & srcRGB24[i * 3];

        uint8_t alpha = (context.mode == BlendMode::COLORINGONLY) * 255 +
            (context.mode != BlendMode::COLORINGONLY) * srcPixel[3];

        uint8_t mask = -(alpha != 0);
        alpha &= mask;

        if (colorFactor) {
            srcColor[0] = (srcColor[0] * colorDataAsRGB[0]) >> 8;
            srcColor[1] = (srcColor[1] * colorDataAsRGB[1]) >> 8;
            srcColor[2] = (srcColor[2] * colorDataAsRGB[2]) >> 8;
            alpha = (alpha * coloring.color.data[0]) >> 8;
        }

        uint8_t srcFactorR, dstFactorR;
        uint8_t srcFactorG, dstFactorG;
        uint8_t srcFactorB, dstFactorB;

        switch (context.colorBlendFactorSrc) {
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
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = srcFactorG = srcFactorB = 0;
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

        switch (context.colorBlendFactorDst) {
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
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = dstFactorG = dstFactorB = 0;
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

        switch (context.colorBlendOperation) {
        case BlendOperation::Add:
            dstPixel[0] = (srcColor[0] * srcFactorR + dstPixel[0] * dstFactorR) >> 8;
            dstPixel[1] = (srcColor[1] * srcFactorG + dstPixel[1] * dstFactorG) >> 8;
            dstPixel[2] = (srcColor[2] * srcFactorB + dstPixel[2] * dstFactorB) >> 8;
            break;
        case BlendOperation::Subtract: {
            int tempR = (srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8;
            int tempG = (srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8;
            int tempB = (srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8;

            dstPixel[0] = tempR < 0 ? 0 : (tempR > 255 ? 255 : tempR);
            dstPixel[1] = tempG < 0 ? 0 : (tempG > 255 ? 255 : tempG);
            dstPixel[2] = tempB < 0 ? 0 : (tempB > 255 ? 255 : tempB);
            break;
        }
        case BlendOperation::ReverseSubtract: {
            int tempR = (dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8;
            int tempG = (dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8;
            int tempB = (dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8;

            dstPixel[0] = tempR < 0 ? 0 : (tempR > 255 ? 255 : tempR);
            dstPixel[1] = tempG < 0 ? 0 : (tempG > 255 ? 255 : tempG);
            dstPixel[2] = tempB < 0 ? 0 : (tempB > 255 ? 255 : tempB);
            break;
        }
        default:
            break;
        }
    }
}