#ifndef SCALETEXTURERENDERER
#define SCALETEXTURERENDERER

#include "../RendererBase.h"
#include "../../data/Texture.h"

namespace Tergos2D
{
    class ScaleTextureRenderer : RendererBase
    {
    public:
        ScaleTextureRenderer(RenderContext2D &context);
        ~ScaleTextureRenderer() = default;


        void DrawTexture(Texture &texture, int16_t x, int16_t y,
                         float scaleX, float scaleY);
        private:

    };

}

#endif 