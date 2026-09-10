#pragma once
#include <array>
#include <memory>
#include <span>
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Lighting;
class W3DRenderObject;
class W3DRenderContext;
class W3DShroud;

// Translates asset geometry and material state; graphics owns mesh resources,
// material shading, and submission. A cache belongs to its game-side owner.
class W3DObjectGraphics final
{
public:
    W3DObjectGraphics();
    ~W3DObjectGraphics();
    W3DObjectGraphics(const W3DObjectGraphics&) = delete;
    W3DObjectGraphics& operator=(const W3DObjectGraphics&) = delete;
    static void Mark_Muzzle_Flash(W3DRenderObject& object);
    void Invalidate();
    bool Render(W3DRenderObject& object, W3DRenderContext& info,
        const Graphics::PropLighting& lighting, W3DShroud* shroud, bool background = false);
private:
    struct State;
    std::unique_ptr<State> m_state;
};
