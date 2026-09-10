#pragma once

#include <memory>

// Adapts editor strings and coordinates to the shared font and text systems.
class ViewportLabels final
{
public:
    ViewportLabels();
    ~ViewportLabels();
    bool Initialize();
    bool Draw(const char* text, int x, int y, unsigned color);

private:
    struct State;
    std::unique_ptr<State> m_state;
};
