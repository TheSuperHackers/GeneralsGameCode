module;
#include "../../profiling/Tracy.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Shroud.Image;
export import Graphics.RHI;

namespace Graphics
{
// CPU-owned projection image. The caller supplies packed 16-bit pixels and
// owns a matching texture. Cells begin one texel inside its padded border.
// Invalidate the upload when the owner resets its device or externally writes
// the texture. CPU pixels remain available for the first draw after recreation.
export class ShroudImage final
{
public:
    bool Set_Cells(std::span<const std::uint16_t> cells, std::uint32_t width,
        std::uint32_t height, std::uint32_t row_stride, std::uint32_t texture_width,
        std::uint32_t texture_height, std::uint16_t border)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Shroud.SetCells");
        const auto count = std::uint64_t(texture_width)*texture_height;
        if (width == 0 || height == 0 || row_stride < width
            || std::uint64_t(width)+2 > texture_width || std::uint64_t(height)+2 > texture_height
            || std::uint64_t(height-1)*row_stride+width > cells.size()
            || count > std::numeric_limits<std::uint32_t>::max()/sizeof(std::uint16_t)) return false;
        if (m_width != texture_width || m_height != texture_height || m_border != border) {
            m_pixels.assign(static_cast<std::size_t>(count),border);
            m_width = texture_width;
            m_height = texture_height;
            m_border = border;
            m_dirty = true;
        } else if (m_cells_width != width || m_cells_height != height) {
            // A smaller map must not leave cells in what is now padding.
            std::fill(m_pixels.begin(),m_pixels.end(),border);
            m_dirty = true;
        }
        m_cells_width = width;
        m_cells_height = height;
        for (std::uint32_t row=0;row<height;++row) {
            auto* destination = m_pixels.data()+std::size_t(row+1)*texture_width+1;
            const auto* source = cells.data()+std::size_t(row)*row_stride;
            const auto bytes = std::size_t(width)*sizeof(std::uint16_t);
            if (std::memcmp(destination,source,bytes) != 0) {
                std::memcpy(destination,source,bytes);
                m_dirty = true;
            }
        }
        return true;
    }

    bool Upload(Device& device, RHITextureHandle texture)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Shroud.Upload");
        if (m_pixels.empty() || !texture.Is_Valid()) return false;
        if (!m_dirty && m_device == &device && m_uploaded_texture == texture) return true;
        if (!device.Update_Texture(texture,{std::as_bytes(std::span(m_pixels)),
            m_width*static_cast<std::uint32_t>(sizeof(std::uint16_t))})) return false;
        m_device = &device;
        m_uploaded_texture = texture;
        m_dirty = false;
        return true;
    }

    void Invalidate_Upload() noexcept
    {
        m_dirty = true;
        m_device = nullptr;
        m_uploaded_texture = {};
    }

private:
    std::vector<std::uint16_t> m_pixels;
    std::uint32_t m_width = 0;
    std::uint32_t m_height = 0;
    std::uint32_t m_cells_width = 0;
    std::uint32_t m_cells_height = 0;
    std::uint16_t m_border = 0;
    bool m_dirty = true;
    Device* m_device = nullptr;
    RHITextureHandle m_uploaded_texture{};
};
}
