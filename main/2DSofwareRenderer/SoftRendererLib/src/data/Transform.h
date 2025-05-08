#include <cstdint>

namespace Tergos2D
{
    struct Scale
    {
        float X, Y = 1;
    };

    class Transform
    {
    public:
        Transform(float rotation);
        Transform(Scale scale);
        Transform(float rotation, Scale scale);
        ~Transform() = default;

        float GetRotation();
        Scale GetScale();

    private:
        float rotation = 0;
        Scale scale;
    };

}