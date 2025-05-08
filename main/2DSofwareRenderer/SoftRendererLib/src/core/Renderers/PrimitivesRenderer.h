#ifndef PRIMITIVESRENDERER_H
#define PRIMITIVESRENDERER_H

#include "../RendererBase.h"
#include "../../data/Color.h"

namespace Tergos2D
{

    class PrimitivesRenderer : RendererBase
    {
    public:
        PrimitivesRenderer(RenderContext2D &context);
        ~PrimitivesRenderer() = default;

        void DrawLine(Color color, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
        void DrawRect(Color color, int16_t x, int16_t y, uint16_t length, uint16_t height);


        void DrawTransformedRect(Color color, uint16_t length, uint16_t height, const float transformationMatrix[3][3]);

        private:
    };

} // namespace Tergos2D

#endif //