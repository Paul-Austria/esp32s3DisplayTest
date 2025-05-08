#include "PixelConverter.h"
#include "PixelFormatInfo.h"
namespace Tergos2D
{

    void PixelConverter::Move(const uint8_t *src, uint8_t *dst, size_t count)
    {
        std::memcpy(dst, src, count * 1);
    }
    void PixelConverter::Move2(const uint8_t *src, uint8_t *dst, size_t count)
    {
        std::memcpy(dst, src, count * 2);
    }
    void PixelConverter::Move3(const uint8_t *src, uint8_t *dst, size_t count)
    {
        std::memcpy(dst, src, count * 3);
    }
    void PixelConverter::Move4(const uint8_t *src, uint8_t *dst, size_t count)
    {
        std::memcpy(dst, src, count * 4);
    }



    PixelConverter::ConvertFunc PixelConverter::GetConversionFunction(PixelFormat from, PixelFormat to)
    {

        if (from == to)
        {
            uint8_t pixelPerByte = PixelFormatRegistry::GetInfo(from).bytesPerPixel;

            switch (pixelPerByte)
            {
            case 4:
                return Move4;
                break;
            case 3:
                return Move3;
                break;
            case 2:
                return Move2;
                break;
            case 1:
                return Move;
                break;

            default:
                break;
            }
        }
        for (const auto &conversion : defaultConversions)
        {
            if (conversion.from == from && conversion.to == to)
            {
                return conversion.func;
            }
        }
        return nullptr;
    }

    void PixelConverter::Convert(PixelFormat from, PixelFormat to, const uint8_t *src, uint8_t *dst, size_t count)
    {
        ConvertFunc func = GetConversionFunction(from, to);
        if(!func) return;
        func(src, dst, count);
    }

} // namespace Tergos2D
