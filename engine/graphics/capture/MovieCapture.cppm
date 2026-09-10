module;
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
export module Graphics.Capture.MovieCapture;
export import Graphics.Capture.FrameCapture;
import Video.Capture.AVIWriter;
namespace Graphics
{
// Recording state is independent of presentation and simulation timing.
// A changed target size starts a new numbered AVI segment.
export class MovieCapture final
{
public:
    bool Start(const std::filesystem::path& base, float rate)
    {
        Stop();
        if (base.empty() || !std::isfinite(rate) || rate < 0 || rate > 1000000)
            return false;
        m_base = base;
        m_rate = rate == 0 ? 1 : rate;
        m_paused = rate == 0;
        m_active = true;
        return true;
    }

    bool Stop() noexcept
    {
        m_active = false;
        m_next = false;
        m_width = m_height = 0;
        return m_writer.Close();
    }

    bool Toggle(const std::filesystem::path& base, float rate)
    {
        return m_active ? Stop() : Start(base, rate);
    }

    void Pause(bool paused) noexcept { m_paused = paused; }
    void Request_Frame() noexcept { m_next = true; }
    bool Is_Active() const noexcept { return m_active; }

    bool Capture(Device& device, const RHIBackbuffer& target, RHITextureFormat format, bool force = false)
    {
        if (!m_active || (m_paused && !m_next && !force))
            return true;
        const auto frame = m_readback.Read(device, target.texture, target.width, target.height, format);
        if (!frame.Is_Valid())
            return Fail();
        if (!m_writer.Is_Open() || m_width != frame.width || m_height != frame.height) {
            if (!m_writer.Close() || !Open_Segment(frame.width, frame.height))
                return Fail();
        }
        if (!m_writer.Append(frame))
            return Fail();
        m_next = false;
        return true;
    }

private:
    bool Fail() noexcept { Stop(); return false; }

    bool Open_Segment(std::uint32_t width, std::uint32_t height)
    {
        std::error_code error;
        for (std::uint64_t index = 0; ; ++index) {
            auto path = m_base;
            path += std::to_string(index) + ".AVI";
            const bool exists = std::filesystem::exists(path, error);
            if (error)
                return false;
            if (exists)
                continue;
            if (!m_writer.Open(path, width, height, m_rate))
                return false;
            m_width = width;
            m_height = height;
            return true;
        }
    }

    FrameCapture m_readback;
    Engine::Video::AVIWriter m_writer;
    std::filesystem::path m_base;
    std::uint32_t m_width = 0, m_height = 0;
    float m_rate = 0;
    bool m_active = false, m_paused = false, m_next = false;
};
}
