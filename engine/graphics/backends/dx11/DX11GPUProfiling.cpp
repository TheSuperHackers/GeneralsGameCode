#define NOMINMAX
#include "DX11GPUProfiling.h"
#if defined(RTS_PROFILE_TRACY)
#include <optional>
#include <tracy/TracyD3D11.hpp>
#endif

namespace Graphics::DX11Detail {
struct GPUProfiler::State final {
#if defined(RTS_PROFILE_TRACY)
    tracy::D3D11Ctx* context;
    std::optional<tracy::D3D11ZoneScope> frame;
    State(ID3D11Device* device, ID3D11DeviceContext* commands)
        : context(tracy::CreateD3D11Context(device,commands)) {}
    ~State()
    {
        frame.reset();
        tracy::DestroyD3D11Context(context);
    }
#endif
};

GPUProfiler::GPUProfiler() = default;
GPUProfiler::~GPUProfiler() = default;

void GPUProfiler::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
#if defined(RTS_PROFILE_TRACY)
    if (!m_state) m_state = std::make_unique<State>(device,context);
#endif
}

void GPUProfiler::Begin_Frame() noexcept
{
#if defined(RTS_PROFILE_TRACY)
    if (!m_state) return;
    static constexpr tracy::SourceLocationData location{
        "Graphics.DX11.GPUFrame",__FUNCTION__,__FILE__,__LINE__,0};
    m_state->frame.emplace(m_state->context,&location,true);
#endif
}

void GPUProfiler::End_Frame() noexcept
{
#if defined(RTS_PROFILE_TRACY)
    if (!m_state) return;
    m_state->frame.reset();
    m_state->context->Collect();
#endif
}
}
