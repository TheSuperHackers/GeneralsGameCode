module;
#include <functional>
#include <utility>
export module Graphics.Frame.ResourceLifecycle;

namespace Graphics
{
export class FrameResourceLifecycle;

// A registration expires before its captured owner is destroyed. Replacing the
// active owner does not restore an earlier registration when the replacement
// expires. All lifecycle operations run on the frame/device thread.
export class FrameResourceRegistration final
{
public:
    FrameResourceRegistration() = default;
    FrameResourceRegistration(const FrameResourceRegistration&) = delete;
    FrameResourceRegistration& operator=(const FrameResourceRegistration&) = delete;
    FrameResourceRegistration(FrameResourceRegistration&& other) noexcept;
    FrameResourceRegistration& operator=(FrameResourceRegistration&& other) noexcept;
    ~FrameResourceRegistration() { Reset(); }
    void Reset() noexcept;

private:
    friend class FrameResourceLifecycle;
    FrameResourceRegistration(FrameResourceLifecycle& lifecycle,
        std::function<void()> release,std::function<void()> reacquire);
    FrameResourceLifecycle* m_lifecycle = nullptr;
    std::function<void()> m_release,m_reacquire;
};

export class FrameResourceLifecycle final
{
public:
    FrameResourceLifecycle() = default;
    FrameResourceLifecycle(const FrameResourceLifecycle&) = delete;
    FrameResourceLifecycle& operator=(const FrameResourceLifecycle&) = delete;
    ~FrameResourceLifecycle() { if (m_active) m_active->m_lifecycle=nullptr; }

    FrameResourceRegistration Register(std::function<void()> release,std::function<void()> reacquire)
    {
        return FrameResourceRegistration(*this,std::move(release),std::move(reacquire));
    }
    void Release()
    {
        // Keep the invocation alive if the callback unregisters its owner.
        const auto callback=m_active ? m_active->m_release : std::function<void()>{};
        if (callback) callback();
    }
    void Reacquire()
    {
        const auto callback=m_active ? m_active->m_reacquire : std::function<void()>{};
        if (callback) callback();
    }

private:
    friend class FrameResourceRegistration;
    FrameResourceRegistration* m_active = nullptr;
};

FrameResourceRegistration::FrameResourceRegistration(FrameResourceLifecycle& lifecycle,
    std::function<void()> release,std::function<void()> reacquire)
    : m_lifecycle(&lifecycle),m_release(std::move(release)),m_reacquire(std::move(reacquire))
{
    if (lifecycle.m_active) lifecycle.m_active->m_lifecycle=nullptr;
    lifecycle.m_active=this;
}

FrameResourceRegistration::FrameResourceRegistration(FrameResourceRegistration&& other) noexcept
    : m_lifecycle(std::exchange(other.m_lifecycle,nullptr)),
      m_release(std::move(other.m_release)),m_reacquire(std::move(other.m_reacquire))
{
    if (m_lifecycle) m_lifecycle->m_active=this;
}

FrameResourceRegistration& FrameResourceRegistration::operator=(FrameResourceRegistration&& other) noexcept
{
    if (this==&other) return *this;
    Reset();
    m_lifecycle=std::exchange(other.m_lifecycle,nullptr);
    m_release=std::move(other.m_release); m_reacquire=std::move(other.m_reacquire);
    if (m_lifecycle) m_lifecycle->m_active=this;
    return *this;
}

void FrameResourceRegistration::Reset() noexcept
{
    if (m_lifecycle) { m_lifecycle->m_active=nullptr; m_lifecycle=nullptr; }
    m_release={}; m_reacquire={};
}

export FrameResourceLifecycle& Get_Frame_Resource_Lifecycle()
{
    static FrameResourceLifecycle lifecycle;
    return lifecycle;
}
}
