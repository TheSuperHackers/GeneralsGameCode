module;
#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <utility>

export module Graphics.Text.GlyphAtlas;

namespace Graphics {
export struct GlyphAtlasRegion final {
    std::uint32_t page = 0;
    std::uint32_t x = 0, y = 0, width = 0, height = 0;
};

export class GlyphAtlas final {
public:
    explicit GlyphAtlas(std::uint32_t page_size = 1024) : m_page_size(page_size) {}
    void Clear() { m_pages.clear(); }
    std::optional<GlyphAtlasRegion> Add(std::uint32_t width, std::uint32_t height,
        std::span<const std::uint8_t> alpha)
    {
        if (!width || !height || m_page_size < 4 || width > m_page_size-2 || height > m_page_size-2
            || alpha.size() < static_cast<std::size_t>(width)*height) return {};
        if (m_pages.empty()) Add_Page();
        Page* page = &m_pages.back();
        if (page->x+width+1 > m_page_size) {
            page->x=1; page->y+=page->row_height+1; page->row_height=0;
        }
        if (page->y+height+1 > m_page_size) { Add_Page(); page=&m_pages.back(); }
        const GlyphAtlasRegion region{static_cast<std::uint32_t>(m_pages.size()-1),page->x,page->y,width,height};
        for (std::uint32_t y=0; y<height; ++y)
            for (std::uint32_t x=0; x<width; ++x) {
                const auto destination=(static_cast<std::size_t>(page->y+y)*m_page_size+page->x+x)*4;
                page->pixels[destination]=255;
                page->pixels[destination+1]=255;
                page->pixels[destination+2]=255;
                page->pixels[destination+3]=alpha[static_cast<std::size_t>(y)*width+x];
            }
        page->x+=width+1;
        page->row_height=std::max(page->row_height,height);
        return region;
    }
    std::uint32_t Page_Size() const noexcept { return m_page_size; }
    std::size_t Page_Count() const noexcept { return m_pages.size(); }
    std::span<const std::uint8_t> Pixels(std::uint32_t page) const noexcept {
        return page<m_pages.size() ? std::span<const std::uint8_t>(m_pages[page].pixels) : std::span<const std::uint8_t>{};
    }
private:
    struct Page {
        std::vector<std::uint8_t> pixels;
        std::uint32_t x=1,y=1,row_height=0;
    };
    void Add_Page() {
        Page page;
        page.pixels.resize(static_cast<std::size_t>(m_page_size)*m_page_size*4);
        m_pages.push_back(std::move(page));
    }
    std::uint32_t m_page_size;
    std::vector<Page> m_pages;
};
}
