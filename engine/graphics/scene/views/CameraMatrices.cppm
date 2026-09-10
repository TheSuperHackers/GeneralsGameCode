module;
#include <cstddef>
export module Graphics.Scene.Views.CameraMatrices;
export import Graphics.Scene.Views.View;

namespace Graphics
{
export struct CameraMatrices final
{
    Matrix4x4 view = Matrix4x4::Identity();
    Matrix4x4 projection = Matrix4x4::Identity();
};

// Camera adapters publish view inputs on the device thread. Object transforms
// belong to individual draw descriptions, never to this traversal context.
export CameraMatrices& Get_Camera_Matrices() noexcept
{
    static CameraMatrices camera;
    return camera;
}
}
