#include "ViewportLabels.h"
#include "Common/UnicodeString.h"

import Assets.Cache;
import Engine.UI.WND;

struct ViewportLabels::State
{
    Assets::FontAssetHandle asset;
    Engine::UI::WND::FontFace face;
};

ViewportLabels::ViewportLabels() = default;
ViewportLabels::~ViewportLabels() = default;

bool ViewportLabels::Initialize()
{
    if (m_state) return true;
    auto* cache = Assets::Try_Get_Asset_Cache();
    if (!cache) return false;
    auto state = std::make_unique<State>();
    // The editor label contract is 20 pixels at 96 DPI, or 15 points.
    state->asset = cache->Request_Font("Arial", 15, false);
    if (!state->asset.Is_Valid()) return false;
    cache->Wait(state->asset);
    const auto* asset = cache->Try_Get_Font(state->asset);
    if (!asset || !state->face.Build(*asset)) return false;
    m_state = std::move(state);
    return true;
}

bool ViewportLabels::Draw(const char* text, int x, int y, unsigned color)
{
    if (!m_state || !text) return false;
    UnicodeString unicode;
    unicode.translate(text);
    static_assert(sizeof(WideChar) == sizeof(std::uint16_t));
    Engine::UI::WND::TextStyle style;
    style.color = {float((color >> 16) & 255) / 255.0f,
        float((color >> 8) & 255) / 255.0f, float(color & 255) / 255.0f,
        float((color >> 24) & 255) / 255.0f};
    style.drop_color = {0, 0, 0, 0};
    return Engine::UI::WND::Get_Text_Renderer().Draw(Graphics::Get_Renderer2D(),
        m_state->face, nullptr, reinterpret_cast<const std::uint16_t*>(unicode.str()),
        float(x), float(y), {}, style);
}
