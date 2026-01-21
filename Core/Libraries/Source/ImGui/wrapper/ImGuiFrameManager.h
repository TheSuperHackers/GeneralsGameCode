#pragma once


namespace ImGui {
    class FrameManager
    {
    public:
        static void BeginFrame();
        static void EndFrame(); // Includes Render()

    private:
        static bool s_frameOpen;
    };
}  // namespace ImGui


