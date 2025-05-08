#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include "PixelFormat/PixelFormat.h"

namespace Tergos2D{

class Texture
{
public:
    Texture() = default;
    Texture(uint16_t width, uint16_t height, PixelFormat format, uint16_t pitch = 0);
    Texture(uint16_t width, uint16_t height, uint8_t* data,PixelFormat format, uint16_t pitch = 0);
    Texture(uint16_t orgWidth,uint16_t orgHeight,uint16_t width, uint16_t height, uint16_t startX, uint16_t startY, uint8_t* data, PixelFormat format, uint16_t pitch = 0, bool useOrigSize= false);
    ~Texture();

    /// @brief Get Pointer of Texture
    /// @return uint8_t*
    uint8_t* GetData();


    /// @brief Get Format of Texture
    /// @return PixelFormat
    PixelFormat GetFormat();


    uint16_t GetPitch();
    uint16_t GetWidth();
    uint16_t GetHeight();

private:
    uint8_t* data;
    PixelFormat format;
    bool isSubTexture = false;
    bool storedLocally = false;
    uint16_t width, height;
    uint16_t pitch = 0;
};

}

#endif