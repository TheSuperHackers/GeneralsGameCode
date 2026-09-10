module;
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <span>
#include <utility>
#include <vector>
export module Assets.Images.Buffer;
export import Assets.Images.PixelEncoding;

namespace Assets
{
export struct ImageDescription final
{
    PixelEncoding encoding=PixelEncoding::Unknown;
    unsigned width=0,height=0;
};

export struct ImageRegion final
{
    int left=0, top=0, right=0, bottom=0;
};

// Mutable preparation storage. Published runtime images remain immutable.
export class ImageBuffer final
{
public:
    static std::optional<ImageBuffer> Create(unsigned width,unsigned height,PixelEncoding encoding)
    {
        if (!width || !height) return {};
        const bool compressed=Is_Block_Compressed(encoding);
        const unsigned stride=compressed ? (encoding==PixelEncoding::BC1 ? 8 : 16) : Pixel_Size(encoding);
        if (!stride) return {};
        const std::size_t columns=compressed ? (std::size_t(width)+3)/4 : width;
        const std::size_t rows=compressed ? (std::size_t(height)+3)/4 : height;
        if (columns>std::numeric_limits<unsigned>::max()/stride) return {};
        const unsigned pitch=static_cast<unsigned>(columns*stride);
        if (rows>std::vector<std::byte>().max_size()/pitch) return {};
        return ImageBuffer(width,height,encoding,pitch,rows*pitch);
    }

    unsigned Width() const noexcept { return m_width; }
    unsigned Height() const noexcept { return m_height; }
    unsigned Row_Pitch() const noexcept { return m_pitch; }
    PixelEncoding Encoding() const noexcept { return m_encoding; }
    ImageDescription Description() const noexcept { return {m_encoding,m_width,m_height}; }
    std::span<std::byte> Bytes() noexcept { return m_bytes; }
    std::span<const std::byte> Bytes() const noexcept { return m_bytes; }
    ImageView View() const noexcept { return {m_bytes,m_width,m_height,m_pitch,m_encoding}; }
    bool Contains(ImageRegion region) const noexcept
    {
        return region.left>=0 && region.top>=0 && region.right>region.left && region.bottom>region.top
            && static_cast<unsigned>(region.right)<=m_width && static_cast<unsigned>(region.bottom)<=m_height;
    }

private:
    ImageBuffer(unsigned width,unsigned height,PixelEncoding encoding,unsigned pitch,std::size_t size)
        : m_width(width),m_height(height),m_pitch(pitch),m_encoding(encoding),m_bytes(size) {}
    unsigned m_width,m_height,m_pitch;
    PixelEncoding m_encoding;
    std::vector<std::byte> m_bytes;
};

// Nearest endpoint-aligned resampling preserves the authored surface-copy rule.
// Validate both rectangles and the conversion before modifying the destination.
export bool Copy_Image_Region(const ImageBuffer& source,ImageRegion source_region,
    ImageBuffer& destination,ImageRegion destination_region)
{
    if (!source.Contains(source_region) || !destination.Contains(destination_region)
        || Is_Block_Compressed(source.Encoding()) || Is_Block_Compressed(destination.Encoding())) return false;
    const unsigned source_stride=Pixel_Size(source.Encoding()),destination_stride=Pixel_Size(destination.Encoding());
    const bool same_encoding=source.Encoding()==destination.Encoding();
    if (!same_encoding) {
        std::array<std::byte,4> probe{};
        if (!Convert_Image_Pixel(probe,destination.Encoding(),probe,source.Encoding())) return false;
    }
    std::vector<std::byte> snapshot;
    std::span<const std::byte> input=source.Bytes();
    if (&source==&destination) { snapshot.assign(input.begin(),input.end()); input=snapshot; }
    const unsigned sw=source_region.right-source_region.left,sh=source_region.bottom-source_region.top;
    const unsigned dw=destination_region.right-destination_region.left,dh=destination_region.bottom-destination_region.top;
    for (unsigned y=0;y<dh;++y) {
        const unsigned sy=source_region.top+(sh==dh ? y : static_cast<unsigned>((float(y)/std::max(1u,dh-1))*(sh-1)));
        if (same_encoding && sw==dw) {
            const auto from=input.subspan(std::size_t(sy)*source.Row_Pitch()+std::size_t(source_region.left)*source_stride,
                std::size_t(sw)*source_stride);
            auto to=destination.Bytes().subspan(std::size_t(destination_region.top+y)*destination.Row_Pitch()
                +std::size_t(destination_region.left)*destination_stride,from.size());
            std::memcpy(to.data(),from.data(),from.size());
            continue;
        }
        for (unsigned x=0;x<dw;++x) {
            const unsigned sx=source_region.left+(sw==dw ? x : static_cast<unsigned>((float(x)/std::max(1u,dw-1))*(sw-1)));
            const auto from=input.subspan(std::size_t(sy)*source.Row_Pitch()+std::size_t(sx)*source_stride,source_stride);
            auto to=destination.Bytes().subspan(std::size_t(destination_region.top+y)*destination.Row_Pitch()
                +std::size_t(destination_region.left+x)*destination_stride,destination_stride);
            if (same_encoding) std::memcpy(to.data(),from.data(),source_stride);
            else if (!Convert_Image_Pixel(to,destination.Encoding(),from,source.Encoding())) return false;
        }
    }
    return true;
}
}
