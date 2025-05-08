#ifndef TRANSFORMEDTEXTURERENDERER_H
#define TRANSFORMEDTEXTURERENDERER_H

#include "../RendererBase.h"
#include "../../data/Color.h"
#include "../../data/Texture.h"
#include <functional>

#define MAX_BUFFER_SIZE 64
namespace Tergos2D
{

    using DrawTexturePointer = void (*)(Texture &texture,  const float transformationMatrix[3][3], RenderContext2D& context, int startX, int StartY, int endX, int endY);


    class TransformedTextureRenderer : RendererBase
    {
    public:
        TransformedTextureRenderer(RenderContext2D &context);
        ~TransformedTextureRenderer() = default;

        /// @brief draw a texture transformed
        /// @param texture
        /// @param transformationMatrix
        /// @param startX
        /// @param StartY
        /// @param endX
        /// @param endY
        void DrawTexture(Texture &texture,  const float transformationMatrix[3][3],
            int startX = 0,
            int StartY = 0,
            int endX = 0,
            int endY = 0);

        /// @brief generic implementation without sampling support
        /// @param texture
        /// @param transformationMatrix
        /// @param context
        static void DrawTexture(Texture &texture,  const float transformationMatrix[3][3], RenderContext2D& context, int startX, int StartY, int endX, int endY);


        /// @brief  Implementation with Sampling Support supports any BytePixel format (RGBA32,RGB24,Grayscale8, etc ..) not formats like rgb565
        /// @param texture
        /// @param transformationMatrix
        /// @param context
        static void DrawTextureSamplingSupp(Texture &texture,  const float transformationMatrix[3][3], RenderContext2D& context, int startX, int StartY, int endX, int endY);


        /// @brief Get the function thats used for drawing a texture transformed
        /// @return
        DrawTexturePointer GetDrawTexture();
        /// @brief Sets the function that is used to transform a texture
        /// @param drawTexture
        void SetDrawTexture(DrawTexturePointer drawTexture);
    private:

        	DrawTexturePointer m_drawTexture = DrawTexture;
    };

} // namespace Tergos2D

#endif //