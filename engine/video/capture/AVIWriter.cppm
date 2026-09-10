module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <vector>
export module Video.Capture.AVIWriter;
export import Video.Frame;
namespace Engine::Video
{
// Uncompressed, indexed AVI output. Frames are top-down RGB/RGBA/BGRA views;
// the writer owns conversion to padded bottom-up BGR rows and container state.
export class AVIWriter final
{
public:
    AVIWriter() = default;
    AVIWriter(const AVIWriter&) = delete;
    AVIWriter& operator=(const AVIWriter&) = delete;
    ~AVIWriter() { Close(); }

    bool Open(const std::filesystem::path& path,std::uint32_t width,
        std::uint32_t height,float frame_rate)
    {
        if (Is_Open() || width==0 || height==0 || !std::isfinite(frame_rate)
            || frame_rate<=0 || frame_rate>1000000) return false;
        const std::uint64_t pitch=(std::uint64_t(width)*3+3)&~std::uint64_t(3);
        const std::uint64_t bytes=pitch*height;
        if (width>32767 || height>32767 || bytes>std::numeric_limits<std::uint32_t>::max()-256) return false;
        m_frame.resize(static_cast<std::size_t>(bytes));
        m_width=width; m_height=height; m_pitch=static_cast<std::uint32_t>(pitch);
        m_rate=frame_rate; m_offsets.clear();
        m_stream.clear();
        m_stream.open(path,std::ios::binary|std::ios::trunc);
        if (!m_stream.is_open()) return false;
        m_position=0;
        FourCC("RIFF"); U32(0); FourCC("AVI ");
        FourCC("LIST"); U32(192); FourCC("hdrl");
        FourCC("avih"); U32(56);
        U32(std::max(1u,static_cast<std::uint32_t>(1000000.0/frame_rate)));
        U32(static_cast<std::uint32_t>(std::min(double(bytes)*frame_rate,double(UINT32_MAX))));
        U32(0); U32(0x10); U32(0); U32(0); U32(1); U32(static_cast<std::uint32_t>(bytes));
        U32(width); U32(height); U32(0); U32(0); U32(0); U32(0);
        FourCC("LIST"); U32(116); FourCC("strl");
        FourCC("strh"); U32(56); FourCC("vids"); U32(0);
        U32(0); U16(0); U16(0); U32(0);
        U32(1000); U32(std::max(1u,static_cast<std::uint32_t>(frame_rate*1000.0f+0.5f)));
        U32(0); U32(0); U32(static_cast<std::uint32_t>(bytes)); U32(UINT32_MAX); U32(0);
        U16(0); U16(0); U16(static_cast<std::uint16_t>(width)); U16(static_cast<std::uint16_t>(height));
        FourCC("strf"); U32(40); U32(40); U32(width); U32(height);
        U16(1); U16(24); U32(0); U32(static_cast<std::uint32_t>(bytes));
        U32(0); U32(0); U32(0); U32(0);
        FourCC("LIST"); U32(4); FourCC("movi");
        if (!m_stream.good()) { m_stream.close(); return false; }
        return true;
    }

    bool Is_Open() const noexcept { return m_stream.is_open(); }
    float Frame_Rate() const noexcept { return m_rate; }
    std::uint32_t Frame_Count() const noexcept { return static_cast<std::uint32_t>(m_offsets.size()); }

    bool Append(const DecodedVideoFrame& frame)
    {
        if (!Is_Open() || !frame.Is_Valid() || frame.width!=m_width || frame.height!=m_height
            || (frame.format!=PixelFormat::RGB8 && frame.format!=PixelFormat::RGBA8
                && frame.format!=PixelFormat::BGRA8 && frame.format!=PixelFormat::BGRX8)) return false;
        // AVI's 32-bit RIFF/index fields must describe the entire finalized file.
        const auto final_size=m_position+8+m_frame.size()+8+(m_offsets.size()+1)*16;
        if (final_size-8>UINT32_MAX) return false;
        const unsigned channels=Bytes_Per_Pixel(frame.format);
        const bool bgr=frame.format==PixelFormat::BGRA8 || frame.format==PixelFormat::BGRX8;
        std::fill(m_frame.begin(),m_frame.end(),std::byte{});
        for (std::uint32_t y=0;y<m_height;++y) {
            const auto* source=frame.pixels.data()+std::size_t(y)*frame.row_pitch;
            auto* destination=m_frame.data()+std::size_t(m_height-1-y)*m_pitch;
            for (std::uint32_t x=0;x<m_width;++x) {
                destination[x*3]=source[x*channels+(bgr ? 0 : 2)];
                destination[x*3+1]=source[x*channels+1];
                destination[x*3+2]=source[x*channels+(bgr ? 2 : 0)];
            }
        }
        const auto offset=static_cast<std::uint32_t>(m_position-220);
        FourCC("00db"); U32(static_cast<std::uint32_t>(m_frame.size()));
        Write(m_frame);
        if (!m_stream.good()) return false;
        m_offsets.push_back(offset);
        return true;
    }

    bool Close() noexcept
    {
        if (!Is_Open()) return true;
        const auto movie_end=m_position;
        FourCC("idx1"); U32(static_cast<std::uint32_t>(m_offsets.size()*16));
        for (const auto offset : m_offsets) {
            FourCC("00db"); U32(0x10); U32(offset); U32(static_cast<std::uint32_t>(m_frame.size()));
        }
        const auto end=m_position;
        Patch(4,static_cast<std::uint32_t>(end-8));
        Patch(48,Frame_Count()); Patch(140,Frame_Count());
        Patch(216,static_cast<std::uint32_t>(movie_end-220));
        m_stream.flush();
        const bool written=m_stream.good();
        m_stream.close();
        return written && !m_stream.fail();
    }

private:
    void Write(std::span<const std::byte> bytes) noexcept
    {
        m_stream.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
        m_position+=bytes.size();
    }
    void FourCC(const char* value) noexcept
    {
        Write({reinterpret_cast<const std::byte*>(value),4});
    }
    void U32(std::uint32_t value) noexcept
    {
        const std::array bytes{std::byte(value),std::byte(value>>8),std::byte(value>>16),std::byte(value>>24)};
        Write(bytes);
    }
    void U16(std::uint16_t value) noexcept
    {
        const std::array bytes{std::byte(value),std::byte(value>>8)};
        Write(bytes);
    }
    void Patch(std::uint32_t offset,std::uint32_t value) noexcept
    {
        const auto end=m_position;
        m_stream.seekp(offset); U32(value);
        m_position=end; m_stream.seekp(static_cast<std::streamoff>(end));
    }
    std::ofstream m_stream;
    std::vector<std::byte> m_frame;
    std::vector<std::uint32_t> m_offsets;
    std::uint64_t m_position=0;
    std::uint32_t m_width=0,m_height=0,m_pitch=0;
    float m_rate=0;
};
}
