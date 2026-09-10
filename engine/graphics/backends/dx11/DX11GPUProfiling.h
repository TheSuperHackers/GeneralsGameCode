#pragma once

#include <d3d11.h>
#include <memory>

namespace Graphics::DX11Detail {
// Backend timestamps cover the submitted frame, including gaps while the CPU
// prepares commands. They measure elapsed GPU timeline time, not GPU occupancy.
class GPUProfiler final {
public:
    GPUProfiler();
    GPUProfiler(const GPUProfiler&) = delete;
    GPUProfiler& operator=(const GPUProfiler&) = delete;
    ~GPUProfiler();
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Begin_Frame() noexcept;
    void End_Frame() noexcept;
private:
    struct State;
    std::unique_ptr<State> m_state;
};
}
