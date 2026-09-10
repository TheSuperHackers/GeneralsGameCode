#pragma once

#if defined(RTS_PROFILE_TRACY)
#include <cstdlib>
#include <tracy/Tracy.hpp>

namespace Graphics::Profiling
{
// Read once at process startup's first scope. Frame timing and rate plots are
// independent of these detailed zones, so observer overhead can be measured.
inline unsigned Scope_Level() noexcept
{
    static const unsigned level = [] {
        const char* value = std::getenv("RTS_GRAPHICS_TRACE_DETAIL");
        if (value != nullptr && value[0] != '\0' && value[1] == '\0') {
            if (value[0] == '0') return 0u;
            if (value[0] == '2') return 2u;
        }
        return 1u;
    }();
    return level;
}
inline bool Detailed_Scopes_Enabled() noexcept { return Scope_Level() == 1; }
inline bool Focused_Scopes_Enabled() noexcept { return Scope_Level() == 2; }
}

#define GRAPHICS_PROFILE_SCOPE(name) \
    ZoneNamedN(TracyConcat(graphics_profile_zone_,__LINE__),name,::Graphics::Profiling::Detailed_Scopes_Enabled())
#define GRAPHICS_PROFILE_FOCUS_SCOPE(name) \
    ZoneNamedN(TracyConcat(graphics_profile_zone_,__LINE__),name,::Graphics::Profiling::Focused_Scopes_Enabled())
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#define GRAPHICS_PROFILE_FOCUS_SCOPE(name) ((void)0)
#endif
