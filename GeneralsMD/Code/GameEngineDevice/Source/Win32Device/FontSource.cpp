#include "Win32Device/FontSource.h"
#include <windows.h>
#include <memory>
#include <type_traits>

std::vector<std::byte> Read_Installed_Font_Source(const std::string& family, bool bold)
{
    // Ask the platform font mapper for the same family/weight used by the game.
    // Only font bytes cross this boundary; glyph rasterization and atlases are
    // owned by the asset and graphics systems.
    const std::unique_ptr<std::remove_pointer_t<HFONT>,decltype(&DeleteObject)> font(
        CreateFontA(-16,0,0,0,bold ? FW_BOLD : FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_TT_ONLY_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
        DEFAULT_PITCH,family.c_str()),DeleteObject);
    if (!font) return {};
    const std::unique_ptr<std::remove_pointer_t<HDC>,decltype(&DeleteDC)> context(CreateCompatibleDC(nullptr),DeleteDC);
    if (!context) return {};
    const HGDIOBJ previous = SelectObject(context.get(),font.get());
    const DWORD size = GetFontData(context.get(),0,0,nullptr,0);
    std::vector<std::byte> bytes;
    if (size != GDI_ERROR && size != 0) {
        bytes.resize(size);
        if (GetFontData(context.get(),0,0,bytes.data(),size) != size) bytes.clear();
    }
    SelectObject(context.get(),previous);
    return bytes;
}
