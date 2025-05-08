#include "../../PixelConverter.h"

using namespace Tergos2D;


void PixelConverter::BGR24ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 1] = src[i * 3 + 2]; // R
        dst[i * 4 + 2] = src[i * 3 + 1]; // G
        dst[i * 4 + 3] = src[i * 3 + 0]; // B
        dst[i * 4 + 0] = 255;            // A
    }
}

void PixelConverter::RGB24ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
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
    for (; i + 3 <= count; i += 4)
    {
        // Process 4 pixels at once
        const uint8_t *src_0 = src + 4 * i;
        const uint8_t *src_1 = src + 4 * (i + 1);
        const uint8_t *src_2 = src + 4 * (i + 2);
        const uint8_t *src_3 = src + 4 * (i + 3);

        // Copy RGB values directly, ignoring the alpha (first byte in ARGB8888)
        dst[3 * i] = src_0[1];     // R (Pixel 0)
        dst[3 * i + 1] = src_0[2]; // G (Pixel 0)
        dst[3 * i + 2] = src_0[3]; // B (Pixel 0)

        dst[3 * (i + 1)] = src_1[1];     // R (Pixel 1)
        dst[3 * (i + 1) + 1] = src_1[2]; // G (Pixel 1)
        dst[3 * (i + 1) + 2] = src_1[3]; // B (Pixel 1)

        dst[3 * (i + 2)] = src_2[1];     // R (Pixel 2)
        dst[3 * (i + 2) + 1] = src_2[2]; // G (Pixel 2)
        dst[3 * (i + 2) + 2] = src_2[3]; // B (Pixel 2)

        dst[3 * (i + 3)] = src_3[1];     // R (Pixel 3)
        dst[3 * (i + 3) + 1] = src_3[2]; // G (Pixel 3)
        dst[3 * (i + 3) + 2] = src_3[3]; // B (Pixel 3)
    }

    // Handle any remaining pixels if count isn't a multiple of 4
    for (; i < count; ++i)
    {
        dst[3 * i] = src[4 * i + 1];     // R
        dst[3 * i + 1] = src[4 * i + 2]; // G
        dst[3 * i + 2] = src[4 * i + 3]; // B
    }
}

void PixelConverter::ARGB8888ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 3 <= count; i += 4)
    {
        // Process 4 pixels at once
        const uint8_t *src_0 = src + 4 * i;
        const uint8_t *src_1 = src + 4 * (i + 1);
        const uint8_t *src_2 = src + 4 * (i + 2);
        const uint8_t *src_3 = src + 4 * (i + 3);

        // Copy RGB values directly, ignoring the alpha (first byte in ARGB8888)
        dst[3 * i + 2] = src_0[1]; // R (Pixel 0)
        dst[3 * i + 1] = src_0[2]; // G (Pixel 0)
        dst[3 * i + 0] = src_0[3]; // B (Pixel 0)

        dst[3 * (i + 1) + 2] = src_1[1]; // R (Pixel 1)
        dst[3 * (i + 1) + 1] = src_1[2]; // G (Pixel 1)
        dst[3 * (i + 1) + 0] = src_1[3]; // B (Pixel 1)

        dst[3 * (i + 2) + 2] = src_2[1]; // R (Pixel 2)
        dst[3 * (i + 2) + 1] = src_2[2]; // G (Pixel 2)
        dst[3 * (i + 2) + 0] = src_2[3]; // B (Pixel 2)

        dst[3 * (i + 3) + 2] = src_3[1]; // R (Pixel 3)
        dst[3 * (i + 3) + 1] = src_3[2]; // G (Pixel 3)
        dst[3 * (i + 3) + 0] = src_3[3]; // B (Pixel 3)
    }

    // Handle any remaining pixels if count isn't a multiple of 4
    for (; i < count; ++i)
    {
        dst[3 * i + 2] = src[4 * i + 1]; // R
        dst[3 * i + 1] = src[4 * i + 2]; // G
        dst[3 * i + 0] = src[4 * i + 3]; // B
    }
}

void PixelConverter::RGB565ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];

        // Extract and scale the red channel (5 bits to 8 bits)
        dst[i * 4 + 1] = (pixel & 0xF800) >> 8;       // R (5 bits -> 8 bits)
        dst[i * 4 + 1] = (dst[i * 4 + 1] * 255) / 31; // Scale to 0-255

        // Extract and scale the green channel (6 bits to 8 bits)
        dst[i * 4 + 2] = (pixel & 0x07E0) >> 3;       // G (6 bits -> 8 bits)
        dst[i * 4 + 2] = (dst[i * 4 + 2] * 255) / 63; // Scale to 0-255

        // Extract and scale the blue channel (5 bits to 8 bits)
        dst[i * 4 + 3] = (pixel & 0x001F) << 3;       // B (5 bits -> 8 bits)
        dst[i * 4 + 3] = (dst[i * 4 + 3] * 255) / 31; // Scale to 0-255

        // Set the alpha channel to 255 (fully opaque)
        dst[i * 4 + 0] = 255; // A
    }
}

void PixelConverter::RGB565ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 3 <= count; i += 4)
    {
        // Process 4 pixels at once
        uint16_t rgb565_0 = (src[2 * i] << 8) | src[2 * i + 1];
        uint16_t rgb565_1 = (src[2 * (i + 1)] << 8) | src[2 * (i + 1) + 1];
        uint16_t rgb565_2 = (src[2 * (i + 2)] << 8) | src[2 * (i + 2) + 1];
        uint16_t rgb565_3 = (src[2 * (i + 3)] << 8) | src[2 * (i + 3) + 1];

        // Extract and scale components for the first 4 pixels
        uint8_t r0 = (rgb565_0 >> 11) & 0x1F, g0 = (rgb565_0 >> 5) & 0x3F, b0 = rgb565_0 & 0x1F;
        uint8_t r1 = (rgb565_1 >> 11) & 0x1F, g1 = (rgb565_1 >> 5) & 0x3F, b1 = rgb565_1 & 0x1F;
        uint8_t r2 = (rgb565_2 >> 11) & 0x1F, g2 = (rgb565_2 >> 5) & 0x3F, b2 = rgb565_2 & 0x1F;
        uint8_t r3 = (rgb565_3 >> 11) & 0x1F, g3 = (rgb565_3 >> 5) & 0x3F, b3 = rgb565_3 & 0x1F;

        // Scale the components (5-bit red, 6-bit green, 5-bit blue) to 8-bit
        dst[3 * i] = (r0 << 3) | (r0 >> 2);     // Red (Pixel 0)
        dst[3 * i + 1] = (g0 << 2) | (g0 >> 4); // Green (Pixel 0)
        dst[3 * i + 2] = (b0 << 3) | (b0 >> 2); // Blue (Pixel 0)

        dst[3 * (i + 1)] = (r1 << 3) | (r1 >> 2);     // Red (Pixel 1)
        dst[3 * (i + 1) + 1] = (g1 << 2) | (g1 >> 4); // Green (Pixel 1)
        dst[3 * (i + 1) + 2] = (b1 << 3) | (b1 >> 2); // Blue (Pixel 1)

        dst[3 * (i + 2)] = (r2 << 3) | (r2 >> 2);     // Red (Pixel 2)
        dst[3 * (i + 2) + 1] = (g2 << 2) | (g2 >> 4); // Green (Pixel 2)
        dst[3 * (i + 2) + 2] = (b2 << 3) | (b2 >> 2); // Blue (Pixel 2)

        dst[3 * (i + 3)] = (r3 << 3) | (r3 >> 2);     // Red (Pixel 3)
        dst[3 * (i + 3) + 1] = (g3 << 2) | (g3 >> 4); // Green (Pixel 3)
        dst[3 * (i + 3) + 2] = (b3 << 3) | (b3 >> 2); // Blue (Pixel 3)
    }

    // Handle any remaining pixels if count isn't a multiple of 4
    for (; i < count; ++i)
    {
        uint16_t rgb565 = (src[2 * i] << 8) | src[2 * i + 1];

        uint32_t r = (rgb565 >> 11) & 0x1F;
        uint32_t g = (rgb565 >> 5) & 0x3F;
        uint32_t b = rgb565 & 0x1F;

        dst[3 * i] = static_cast<uint8_t>((r << 3) | (r >> 2));
        dst[3 * i + 1] = static_cast<uint8_t>((g << 2) | (g >> 4));
        dst[3 * i + 2] = static_cast<uint8_t>((b << 3) | (b >> 2));
    }
}

void PixelConverter::RGB565ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    size_t i = 0;
    for (; i + 3 <= count; i += 4)
    {
        // Process 4 pixels at once
        uint16_t rgb565_0 = (src[2 * i] << 8) | src[2 * i + 1];
        uint16_t rgb565_1 = (src[2 * (i + 1)] << 8) | src[2 * (i + 1) + 1];
        uint16_t rgb565_2 = (src[2 * (i + 2)] << 8) | src[2 * (i + 2) + 1];
        uint16_t rgb565_3 = (src[2 * (i + 3)] << 8) | src[2 * (i + 3) + 1];

        // Extract and scale components for the first 4 pixels
        uint8_t r0 = (rgb565_0 >> 11) & 0x1F, g0 = (rgb565_0 >> 5) & 0x3F, b0 = rgb565_0 & 0x1F;
        uint8_t r1 = (rgb565_1 >> 11) & 0x1F, g1 = (rgb565_1 >> 5) & 0x3F, b1 = rgb565_1 & 0x1F;
        uint8_t r2 = (rgb565_2 >> 11) & 0x1F, g2 = (rgb565_2 >> 5) & 0x3F, b2 = rgb565_2 & 0x1F;
        uint8_t r3 = (rgb565_3 >> 11) & 0x1F, g3 = (rgb565_3 >> 5) & 0x3F, b3 = rgb565_3 & 0x1F;

        // Scale the components (5-bit red, 6-bit green, 5-bit blue) to 8-bit
        dst[3 * i + 2] = (r0 << 3) | (r0 >> 2); // Red (Pixel 0)
        dst[3 * i + 1] = (g0 << 2) | (g0 >> 4); // Green (Pixel 0)
        dst[3 * i + 0] = (b0 << 3) | (b0 >> 2); // Blue (Pixel 0)

        dst[3 * (i + 1) + 2] = (r1 << 3) | (r1 >> 2); // Red (Pixel 1)
        dst[3 * (i + 1) + 1] = (g1 << 2) | (g1 >> 4); // Green (Pixel 1)
        dst[3 * (i + 1) + 0] = (b1 << 3) | (b1 >> 2); // Blue (Pixel 1)

        dst[3 * (i + 2) + 2] = (r2 << 3) | (r2 >> 2); // Red (Pixel 2)
        dst[3 * (i + 2) + 1] = (g2 << 2) | (g2 >> 4); // Green (Pixel 2)
        dst[3 * (i + 2) + 0] = (b2 << 3) | (b2 >> 2); // Blue (Pixel 2)

        dst[3 * (i + 3) + 2] = (r3 << 3) | (r3 >> 2); // Red (Pixel 3)
        dst[3 * (i + 3) + 1] = (g3 << 2) | (g3 >> 4); // Green (Pixel 3)
        dst[3 * (i + 3) + 0] = (b3 << 3) | (b3 >> 2); // Blue (Pixel 3)
    }

    // Handle any remaining pixels if count isn't a multiple of 4
    for (; i < count; ++i)
    {
        uint16_t rgb565 = (src[2 * i] << 8) | src[2 * i + 1];

        uint32_t r = (rgb565 >> 11) & 0x1F;
        uint32_t g = (rgb565 >> 5) & 0x3F;
        uint32_t b = rgb565 & 0x1F;

        dst[3 * i + 2] = static_cast<uint8_t>((r << 3) | (r >> 2));
        dst[3 * i + 1] = static_cast<uint8_t>((g << 2) | (g >> 4));
        dst[3 * i + 0] = static_cast<uint8_t>((b << 3) | (b >> 2));
    }
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
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 1] = (pixel & 0x7C00) >> 7;      // R
        dst[i * 4 + 2] = (pixel & 0x03E0) >> 2;      // G
        dst[i * 4 + 3] = (pixel & 0x001F) << 3;      // B
        dst[i * 4 + 0] = (pixel & 0x8000) ? 255 : 0; // A
    }
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


void PixelConverter::BGR24ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 0] = src[i * 3 + 2]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 0]; // B
        dst[i * 4 + 3] = 255;            // A
    }
}

void PixelConverter::RGB24ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 0] = src[i * 3 + 0]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 2]; // B
        dst[i * 4 + 3] = 255;            // A
    }
}

void PixelConverter::RGB565ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 0] = (pixel & 0xF800) >> 8; // R
        dst[i * 4 + 1] = (pixel & 0x07E0) >> 3; // G
        dst[i * 4 + 2] = (pixel & 0x001F) << 3; // B
        dst[i * 4 + 3] = 255;                   // A
    }
}

void PixelConverter::ARGB1555ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 0] = (pixel & 0x7C00) >> 7;      // R
        dst[i * 4 + 1] = (pixel & 0x03E0) >> 2;      // G
        dst[i * 4 + 2] = (pixel & 0x001F) << 3;      // B
        dst[i * 4 + 3] = (pixel & 0x8000) ? 255 : 0; // A
    }
}

void PixelConverter::RGBA4444ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 0] = (pixel & 0xF000) >> 8; // R
        dst[i * 4 + 1] = (pixel & 0x0F00) >> 4; // G
        dst[i * 4 + 2] = (pixel & 0x00F0);      // B
        dst[i * 4 + 3] = (pixel & 0x000F) << 4; // A
    }
}



void PixelConverter::ARGB8888ToARGB1555(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t r = (src[i * 4 + 1] >> 3) & 0x1F;
        uint16_t g = (src[i * 4 + 2] >> 3) & 0x1F;
        uint16_t b = (src[i * 4 + 3] >> 3) & 0x1F;
        uint16_t a = (src[i * 4 + 0] >= 128) ? 0x8000 : 0;
        reinterpret_cast<uint16_t *>(dst)[i] = a | (r << 10) | (g << 5) | b;
    }
}

void PixelConverter::RGBA4444ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 1] = (pixel & 0xF000) >> 8; // R
        dst[i * 4 + 2] = (pixel & 0x0F00) >> 4; // G
        dst[i * 4 + 3] = (pixel & 0x00F0);      // B
        dst[i * 4 + 0] = (pixel & 0x000F) << 4; // A
    }
}

void PixelConverter::ARGB8888ToRGBA4444(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t r = (src[i * 4 + 1] >> 4) & 0x0F;
        uint16_t g = (src[i * 4 + 2] >> 4) & 0x0F;
        uint16_t b = (src[i * 4 + 3] >> 4) & 0x0F;
        uint16_t a = (src[i * 4 + 0] >> 4) & 0x0F;
        reinterpret_cast<uint16_t *>(dst)[i] = (r << 12) | (g << 8) | (b << 4) | a;
    }
}


void PixelConverter::ARGB8888ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = static_cast<uint8_t>(0.299f * src[i * 4 + 1] +
                                      0.587f * src[i * 4 + 2] +
                                      0.114f * src[i * 4 + 3]);
    }
}

void PixelConverter::RGB24ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = static_cast<uint8_t>(0.299f * src[i * 3 + 1] +
                                      0.587f * src[i * 3 + 2] +
                                      0.114f * src[i * 3 + 3]);
    }
}

void PixelConverter::RGB565ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        uint8_t r = (pixel & 0xF800) >> 8;
        uint8_t g = (pixel & 0x07E0) >> 3;
        uint8_t b = (pixel & 0x001F) << 3;
        dst[i] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
    }
}

void Tergos2D::PixelConverter::RGBA8888ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 0] = src[i * 4 + 3]; // A
        dst[i * 4 + 1] = src[i * 4 + 0]; // R
        dst[i * 4 + 2] = src[i * 4 + 1]; // G
        dst[i * 4 + 3] = src[i * 4 + 2]; // B
    }
}
void Tergos2D::PixelConverter::ARGB8888ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 3] = src[i * 4 + 0]; // A
        dst[i * 4 + 0] = src[i * 4 + 1]; // R
        dst[i * 4 + 1] = src[i * 4 + 2]; // G
        dst[i * 4 + 2] = src[i * 4 + 3]; // B
    }
}
void PixelConverter::RGBA8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3 + 0] = src[i * 4 + 0]; // R
        dst[i * 3 + 1] = src[i * 4 + 1]; // G
        dst[i * 3 + 2] = src[i * 4 + 2]; // B
    }
}

void PixelConverter::RGBA8888ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3 + 2] = src[i * 4 + 0]; // R
        dst[i * 3 + 1] = src[i * 4 + 1]; // G
        dst[i * 3 + 0] = src[i * 4 + 2]; // B
    }
}

void PixelConverter::RGB24ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3 + 2] = src[i * 3 + 0]; // R
        dst[i * 3 + 1] = src[i * 3 + 1]; // G
        dst[i * 3 + 0] = src[i * 3 + 2]; // B
    }
}


void PixelConverter::BGR24ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3 + 0] = src[i * 3 + 2]; // R
        dst[i * 3 + 1] = src[i * 3 + 1]; // G
        dst[i * 3 + 2] = src[i * 3 + 0]; // B
    }
}


void PixelConverter::Grayscale8ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint8_t gray = src[i];

        dst[i*4]       = gray == 0;
        dst[i * 4 +1]  = gray;
        dst[i * 4 + 2] = gray;
        dst[i * 4 + 3] = gray;
    }
}

void PixelConverter::Grayscale8ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3] =  src[i];
        dst[i * 3 + 1] =  src[i];
        dst[i * 3 + 2] =  src[i];
    }
}
void PixelConverter::Grayscale8ToBGR24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3] =  src[i];
        dst[i * 3 + 1] =  src[i];
        dst[i * 3 + 2] = src[i];
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