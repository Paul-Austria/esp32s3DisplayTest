#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include <stdint.h>
#include "../data/Texture.h"
#include "../data/Color.h"
#include "../data/BlendMode/BlendMode.h"
#include "../data/BlendMode/BlendFunctions.h"
#include "Renderers/PrimitivesRenderer.h"
#include "Renderers/BasicTextureRenderer.h"
#include "Renderers/TransformedTextureRenderer.h"
#include "Renderers/ScaleTextureRenderer.h"


#define MAXBYTESPERPIXEL 4
#define MAXROWLENGTH 500


namespace Tergos2D
{
    enum class SamplingMethod
    {
        NEAREST,
        LINEAR
    };

    struct ClippingArea{
        int16_t startX, startY, endX, endY;
    };

    class RenderContext2D
    {

    public:
        RenderContext2D();
        ~RenderContext2D() = default;

        PrimitivesRenderer primitivesRenderer;
        BasicTextureRenderer basicTextureRenderer;
        TransformedTextureRenderer transformedTextureRenderer;
        ScaleTextureRenderer scaleTextureRenderer;



        void SetTargetTexture(Texture *targetTexture);
        Texture* GetTargetTexture();

        //Determenes if for example a texture not having alpha still needs to blend when coloring is enabled for example,
        BlendMode BlendModeToUse(const PixelFormatInfo& info);

        void SetSamplingMethod(SamplingMethod method);
        SamplingMethod GetSamplingMethod();


        void ClearTarget(Color color);
        void EnableClipping(bool clipping);
        bool IsClippingEnabled();
        void SetClipping(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);

    	ClippingArea GetClippingArea();

        void SetColoringSettings(Coloring coloring);
        Coloring& GetColoring();

        void SetBlendFunc(BlendFunc blendFunc);
        BlendFunc GetBlendFunc();

        BlendContext& GetBlendContext();
        void SetBlendContext(BlendContext context);

    private:
        Texture *targetTexture = nullptr;
        BlendContext m_BlendContext = BlendContext();
        SamplingMethod samplingMethod = SamplingMethod::NEAREST;

        Coloring colorOverlay;
        BlendFunc blendFunc = BlendFunctions::BlendRow;
        // clipping area
        ClippingArea clippingArea;
        bool enableClipping = false;
    };
}
#endif