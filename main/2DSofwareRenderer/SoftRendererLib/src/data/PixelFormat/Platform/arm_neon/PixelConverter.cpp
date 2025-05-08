#include "../../PixelConverter.h"
#include <arm_neon.h>
#include <cstdint>
#include <cstddef>
#include <cstdint>
#include <cstddef>


using namespace Tergos2D;




void PixelConverter::BGR24ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x3_t bgr = vld3_u8(src + i * 3);
        uint8x8_t alpha = vdup_n_u8(255);

        uint8x8x4_t argb;
        argb.val[0] = alpha;
        argb.val[1] = bgr.val[2];
        argb.val[2] = bgr.val[1];
        argb.val[3] = bgr.val[0];

        vst4_u8(dst + i * 4, argb);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 4 + 1] = src[i * 3 + 2]; // R
        dst[i * 4 + 2] = src[i * 3 + 1]; // G
        dst[i * 4 + 3] = src[i * 3 + 0]; // B
        dst[i * 4 + 0] = 255;            // A
    }
}

void PixelConverter::RGB24ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x3_t rgb = vld3_u8(src + i * 3);
        uint8x8_t alpha = vdup_n_u8(255);

        uint8x8x4_t argb;
        argb.val[0] = alpha;
        argb.val[1] = rgb.val[0];
        argb.val[2] = rgb.val[1];
        argb.val[3] = rgb.val[2];

        vst4_u8(dst + i * 4, argb);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 4 + 1] = src[i * 3 + 0]; // R
        dst[i * 4 + 2] = src[i * 3 + 1]; // G
        dst[i * 4 + 3] = src[i * 3 + 2]; // B
        dst[i * 4 + 0] = 255;            // A
    }
}

void PixelConverter::ARGB8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 4 <= count; i += 4)
    {
        uint8x16x4_t argb = vld4q_u8(src + i * 4);

        uint8x16x3_t rgb;
        rgb.val[0] = argb.val[1];
        rgb.val[1] = argb.val[2];
        rgb.val[2] = argb.val[3];

        vst3q_u8(dst + i * 3, rgb);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 3] = src[i * 4 + 1];     // R
        dst[i * 3 + 1] = src[i * 4 + 2]; // G
        dst[i * 3 + 2] = src[i * 4 + 3]; // B
    }
}

void PixelConverter::ARGB8888ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 4 <= count; i += 4)
    {
        uint8x16x4_t argb = vld4q_u8(src + i * 4);

        uint8x16x3_t bgr;
        bgr.val[0] = argb.val[3];
        bgr.val[1] = argb.val[2];
        bgr.val[2] = argb.val[1];

        vst3q_u8(dst + i * 3, bgr);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 3 + 2] = src[i * 4 + 1]; // R
        dst[i * 3 + 1] = src[i * 4 + 2]; // G
        dst[i * 3 + 0] = src[i * 4 + 3]; // B
    }
}

void PixelConverter::RGB565ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void PixelConverter::RGB565ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void PixelConverter::RGB565ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void PixelConverter::ARGB8888ToRGB565(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t r = (src[i * 4 + 1] >> 3) & 0x1F;
        uint16_t g = (src[i * 4 + 2] >> 2) & 0x3F;
        uint16_t b = (src[i * 4 + 3] >> 3) & 0x1F;
        reinterpret_cast<uint16_t *>(dst)[i] = (r << 11) | (g << 5) | b;
    }
}


void PixelConverter::ARGB1555ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
}
void PixelConverter::BGR24ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x3_t bgr = vld3_u8(src + i * 3);
        uint8x8_t alpha = vdup_n_u8(255);

        uint8x8x4_t rgba;
        rgba.val[0] = bgr.val[2];
        rgba.val[1] = bgr.val[1];
        rgba.val[2] = bgr.val[0];
        rgba.val[3] = alpha;

        vst4_u8(dst + i * 4, rgba);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 4 + 0] = src[i * 3 + 2]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 0]; // B
        dst[i * 4 + 3] = 255;            // A
    }
}

void PixelConverter::RGB24ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x3_t rgb = vld3_u8(src + i * 3);
        uint8x8_t alpha = vdup_n_u8(255);

        uint8x8x4_t rgba;
        rgba.val[0] = rgb.val[0];
        rgba.val[1] = rgb.val[1];
        rgba.val[2] = rgb.val[2];
        rgba.val[3] = alpha;

        vst4_u8(dst + i * 4, rgba);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 4 + 0] = src[i * 3 + 0]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 2]; // B
        dst[i * 4 + 3] = 255;            // A
    }
}


void PixelConverter::RGB565ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void PixelConverter::ARGB1555ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
}


void PixelConverter::ARGB1555ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 3 + 0] = (pixel & 0x7C00) >> 7;  // R
        dst[i * 3 + 1] = (pixel & 0x03E0) >> 2;  // G
        dst[i * 3 + 2] = (pixel & 0x001F) << 3;  // B
    }
}

void PixelConverter::ARGB1555ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 3 + 0] = (pixel & 0x001F) << 3;  // B
        dst[i * 3 + 1] = (pixel & 0x03E0) >> 2;  // G
        dst[i * 3 + 2] = (pixel & 0x7C00) >> 7;  // R
    }
}

void PixelConverter::RGBA4444ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void PixelConverter::ARGB8888ToARGB1555(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void PixelConverter::RGBA4444ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void PixelConverter::ARGB8888ToRGBA4444(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void PixelConverter::ARGB8888ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x4_t argb = vld4_u8(src + i * 4);

        uint8x8_t gray = vadd_u8(vadd_u8(vmul_u8(argb.val[1], vdup_n_u8(77)),
                                          vmul_u8(argb.val[2], vdup_n_u8(150))),
                                 vmul_u8(argb.val[3], vdup_n_u8(29)));

        vst1_u8(dst + i, gray);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i] = static_cast<uint8_t>(0.299f * src[i * 4 + 1] +
                                      0.587f * src[i * 4 + 2] +
                                      0.114f * src[i * 4 + 3]);
    }
}

void PixelConverter::RGB24ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x3_t rgb = vld3_u8(src + i * 3);

        uint8x8_t gray = vadd_u8(vadd_u8(vmul_u8(rgb.val[0], vdup_n_u8(77)),
                                          vmul_u8(rgb.val[1], vdup_n_u8(150))),
                                 vmul_u8(rgb.val[2], vdup_n_u8(29)));

        vst1_u8(dst + i, gray);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i] = static_cast<uint8_t>(0.299f * src[i * 3 + 0] +
                                      0.587f * src[i * 3 + 1] +
                                      0.114f * src[i * 3 + 2]);
    }
}

void PixelConverter::RGB565ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
}

void Tergos2D::PixelConverter::RGBA8888ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x4_t rgba = vld4_u8(src + i * 4);

        uint8x8x4_t argb;
        argb.val[0] = rgba.val[3];
        argb.val[1] = rgba.val[0];
        argb.val[2] = rgba.val[1];
        argb.val[3] = rgba.val[2];

        vst4_u8(dst + i * 4, argb);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 4 + 0] = src[i * 4 + 3]; // A
        dst[i * 4 + 1] = src[i * 4 + 0]; // R
        dst[i * 4 + 2] = src[i * 4 + 1]; // G
        dst[i * 4 + 3] = src[i * 4 + 2]; // B
    }
}

void Tergos2D::PixelConverter::ARGB8888ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x4_t argb = vld4_u8(src + i * 4);

        uint8x8x4_t rgba;
        rgba.val[0] = argb.val[1];
        rgba.val[1] = argb.val[2];
        rgba.val[2] = argb.val[3];
        rgba.val[3] = argb.val[0];

        vst4_u8(dst + i * 4, rgba);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 4 + 3] = src[i * 4 + 0]; // A
        dst[i * 4 + 0] = src[i * 4 + 1]; // R
        dst[i * 4 + 1] = src[i * 4 + 2]; // G
        dst[i * 4 + 2] = src[i * 4 + 3]; // B
    }
}


void PixelConverter::RGBA8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x4_t rgba = vld4_u8(src + i * 4);
        uint8x8x3_t rgb;
        rgb.val[0] = rgba.val[0];
        rgb.val[1] = rgba.val[1];
        rgb.val[2] = rgba.val[2];
        vst3_u8(dst + i * 3, rgb);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 3 + 0] = src[i * 4 + 0]; // R
        dst[i * 3 + 1] = src[i * 4 + 1]; // G
        dst[i * 3 + 2] = src[i * 4 + 2]; // B
    }
}

void PixelConverter::RGBA8888ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x4_t rgba = vld4_u8(src + i * 4);
        uint8x8x3_t bgr;
        bgr.val[0] = rgba.val[2];
        bgr.val[1] = rgba.val[1];
        bgr.val[2] = rgba.val[0];
        vst3_u8(dst + i * 3, bgr);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 3 + 2] = src[i * 4 + 0]; // R
        dst[i * 3 + 1] = src[i * 4 + 1]; // G
        dst[i * 3 + 0] = src[i * 4 + 2]; // B
    }
}

void PixelConverter::RGB24ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x3_t rgb = vld3_u8(src + i * 3);
        uint8x8x3_t bgr;
        bgr.val[0] = rgb.val[2];
        bgr.val[1] = rgb.val[1];
        bgr.val[2] = rgb.val[0];
        vst3_u8(dst + i * 3, bgr);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 3 + 2] = src[i * 3 + 0]; // R
        dst[i * 3 + 1] = src[i * 3 + 1]; // G
        dst[i * 3 + 0] = src[i * 3 + 2]; // B
    }
}

void PixelConverter::BGR24ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8x3_t bgr = vld3_u8(src + i * 3);
        uint8x8x3_t rgb;
        rgb.val[0] = bgr.val[2];
        rgb.val[1] = bgr.val[1];
        rgb.val[2] = bgr.val[0];
        vst3_u8(dst + i * 3, rgb);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        dst[i * 3 + 0] = src[i * 3 + 2]; // R
        dst[i * 3 + 1] = src[i * 3 + 1]; // G
        dst[i * 3 + 2] = src[i * 3 + 0]; // B
    }
}

void PixelConverter::Grayscale8ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8_t gray = vld1_u8(src + i);
        uint8x8x4_t argb;
        argb.val[0] = gray;
        argb.val[1] = gray;
        argb.val[2] = gray;
        argb.val[3] = vdup_n_u8(255);
        vst4_u8(dst + i * 4, argb);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        uint8_t gray = src[i];
        dst[i * 4 + 0] = 255; // A
        dst[i * 4 + 1] = gray; // R
        dst[i * 4 + 2] = gray; // G
        dst[i * 4 + 3] = gray; // B
    }
}

void PixelConverter::Grayscale8ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8_t gray = vld1_u8(src + i);
        uint8x8x3_t rgb;
        rgb.val[0] = gray;
        rgb.val[1] = gray;
        rgb.val[2] = gray;
        vst3_u8(dst + i * 3, rgb);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        uint8_t gray = src[i];
        dst[i * 3 + 0] = gray; // R
        dst[i * 3 + 1] = gray; // G
        dst[i * 3 + 2] = gray; // B
    }
}

void PixelConverter::Grayscale8ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 8 <= count; i += 8)
    {
        uint8x8_t gray = vld1_u8(src + i);
        uint8x8x3_t bgr;
        bgr.val[0] = gray;
        bgr.val[1] = gray;
        bgr.val[2] = gray;
        vst3_u8(dst + i * 3, bgr);
    }

    // Handle remaining pixels
    for (; i < count; ++i)
    {
        uint8_t gray = src[i];
        dst[i * 3 + 0] = gray; // B
        dst[i * 3 + 1] = gray; // G
        dst[i * 3 + 2] = gray; // R
    }
}

void Tergos2D::PixelConverter::RGBA8888ToRGB565(const uint8_t * src, uint8_t * dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint8_t r = src[i * 4];
        uint8_t g = src[i * 4 + 1];
        uint8_t b = src[i * 4 + 2];

        uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

        dst[i * 2] = (rgb565 >> 8) & 0xFF;
        dst[i * 2 + 1] = rgb565 & 0xFF;
    }
}

void Tergos2D::PixelConverter::RGB24ToRGB565(const uint8_t * src, uint8_t * dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint8_t r = src[i * 3];
        uint8_t g = src[i * 3 + 1];
        uint8_t b = src[i * 3 + 2];

        uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

        dst[i * 2] = (rgb565 >> 8) & 0xFF;
        dst[i * 2 + 1] = rgb565 & 0xFF;
    }
}

void PixelConverter::BGR24ToRGB565(const uint8_t * src, uint8_t * dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint8_t b = src[i * 3];
        uint8_t g = src[i * 3 + 1];
        uint8_t r = src[i * 3 + 2];

        uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

        dst[i * 2] = (rgb565 >> 8) & 0xFF;
        dst[i * 2 + 1] = rgb565 & 0xFF;
    }
}

void Tergos2D::PixelConverter::Grayscale8ToRGB565(const uint8_t * src, uint8_t * dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint8_t gray = src[i];

        uint16_t rgb565 = ((gray & 0xF8) << 8) | ((gray & 0xFC) << 3) | (gray >> 3);

        dst[i * 2] = (rgb565 >> 8) & 0xFF;
        dst[i * 2 + 1] = rgb565 & 0xFF;
    }
}