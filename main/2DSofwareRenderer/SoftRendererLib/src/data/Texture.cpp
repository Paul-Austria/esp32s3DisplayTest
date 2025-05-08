#include "Texture.h"
#include "PixelFormat/PixelFormatInfo.h"
using namespace Tergos2D;

Texture::Texture(uint16_t width, uint16_t height, PixelFormat format, uint16_t pitch) : pitch(pitch), width(width), height(height), format(format)
{
    storedLocally = true;
    uint8_t bytesPerPixel = PixelFormatRegistry::GetInfo(format).bytesPerPixel;
    if (pitch == 0)
    {
        this->pitch = width * PixelFormatRegistry::GetInfo(format).bytesPerPixel;
    }
    data = new uint8_t[bytesPerPixel * width * height * bytesPerPixel];
}

Texture::Texture(uint16_t width, uint16_t height,
     uint8_t *data, PixelFormat format, uint16_t pitch) : pitch(pitch), width(width), height(height), format(format), data(data)
{
    if (pitch == 0)
    {
        this->pitch = width * PixelFormatRegistry::GetInfo(format).bytesPerPixel;
    }
    storedLocally = false;
}

Texture::Texture(uint16_t orgWidth, uint16_t orgHeight, uint16_t width, uint16_t height,
    uint16_t startX, uint16_t startY, uint8_t* data, PixelFormat format, uint16_t sourcePitch, bool useOrigSize)
    : format(format), storedLocally(false)
{
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(format);

    // Calculate the pitch for the new texture if not provided
    if (sourcePitch == 0)
    {
        this->pitch = (orgWidth * targetInfo.bytesPerPixel);
    }else{
        this->pitch = sourcePitch;
    }

    if(useOrigSize)
    {
        this->width = orgWidth;
        this->height = orgHeight;
    }else{
        this->width =width;
        this->height = height;
    }

    // Calculate the offset for the subtexture
    uint32_t offset = (startY * this->pitch) + (startX * targetInfo.bytesPerPixel);
    this->data = data + offset;
}

Texture::~Texture()
{
    if (storedLocally)
        delete[] data;
}


uint8_t *Texture::GetData()
{
    return data;
}

PixelFormat Texture::GetFormat()
{
    return format;
}

uint16_t Texture::GetWidth()
{
    return width;
}

uint16_t Texture::GetHeight()
{
    return height;
}

uint16_t Texture::GetPitch()
{
    return pitch;
}
