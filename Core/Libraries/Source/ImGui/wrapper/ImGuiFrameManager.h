#pragma once


class ImGuiFrameManager
{
public:
    static void BeginFrame();
    static void EndFrame(); // Includes Render()

private:
    static bool s_frameOpen;
};

