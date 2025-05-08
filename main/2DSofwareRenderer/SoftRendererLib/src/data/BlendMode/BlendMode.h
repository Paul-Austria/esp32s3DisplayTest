#ifndef BLENDMODE
#define BLENDMODE

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"

namespace Tergos2D
{
    enum class BlendMode
    {
        NOBLEND,
        COLORINGONLY,
        BLEND

    };

    enum class BlendFactor
    {
        Zero,
        One,
        SourceAlpha,
        InverseSourceAlpha,
        DestAlpha,
        InverseDestAlpha,
        SourceColor,
        DestColor,
        InverseSourceColor,
        InverseDestColor,
    };

    enum class BlendOperation
    {
        Add, // currently only add supported
        Subtract,
        ReverseSubtract,
        BitwiseAnd,
    };

    struct BlendContext
    {
        BlendMode mode = BlendMode::BLEND;
        BlendFactor colorBlendFactorSrc = BlendFactor::SourceAlpha;
        BlendFactor colorBlendFactorDst = BlendFactor::InverseSourceAlpha;
        BlendOperation colorBlendOperation = BlendOperation::Add;

        BlendFactor alphaBlendFactorSrc = BlendFactor::One;
        BlendFactor alphaBlendFactorDst = BlendFactor::Zero;
        BlendOperation alphaBlendOperation = BlendOperation::Add;
    };

    struct Coloring
    {
        bool colorEnabled = false;
        Color color;
    };

}

#endif