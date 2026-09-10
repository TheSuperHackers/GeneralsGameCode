module;

#include <cstdint>
#include <chrono>
#include <functional>
#include <memory>
#include <utility>

export module Graphics.Resources.Textures.Residency;

export import Graphics.Resources.Loading.Queue;
export import Graphics.Resources.Recreation;
export import Graphics.Resources.Textures.Resource;

namespace Graphics
{

// The caller supplies the simulation or engine clock used for cache ageing.
// Keeping the clock at this boundary avoids coupling resource policy to a
// particular game runtime while retaining the original unsigned wraparound
// subtraction semantics.
export using TextureResidencyClock = std::function<std::uint32_t()>;

export std::uint32_t Texture_Residency_Time_Milliseconds() noexcept
{
    static const auto origin = std::chrono::steady_clock::now();
    return static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - origin).count());
}

// Owns the current native texture generation and the state needed to make a
// texture resident again. File names, archive readers, and image policy stay
// with the owner that supplies the callbacks.
export class TextureResidency final
{
public:
    using Initialize = std::function<void()>;
    using Recreate = std::function<bool()>;

    explicit TextureResidency(TextureResidencyClock clock = {})
        : m_clock(std::move(clock))
    {
        if (!m_clock)
            m_clock = [] { return Texture_Residency_Time_Milliseconds(); };
    }

    TextureResidency(const TextureResidency&) = delete;
    TextureResidency& operator=(const TextureResidency&) = delete;

    ~TextureResidency()
    {
        m_recreation_registration.Reset();
        m_load_source.reset();
        Set_Resource(nullptr);
    }

    void Set_Load_Source(std::shared_ptr<const ResourceLoadSource> source)
    {
        m_load_source = std::move(source);
    }

    const std::shared_ptr<const ResourceLoadSource>& Load_Source() const noexcept
    {
        return m_load_source;
    }

    void Set_Initialize_Callback(Initialize callback)
    {
        m_initialize = std::move(callback);
    }

    void Set_Recreate_Callback(Recreate callback)
    {
        m_recreate = std::move(callback);
    }

    // Registration is explicit because editable snapshots historically did
    // not participate in device-resource recreation. The owner enables it
    // only for resource kinds that have a valid recreation path.
    void Register_For_Recreation()
    {
        if (m_recreation_registered)
            return;

        m_recreation_registration = Get_Resource_Recreation_Registry().Register(
            [this] { Set_Resource(nullptr); },
            [this] { Ensure(); });
        m_recreation_registered = true;
    }

    bool Recreation_Registered() const noexcept
    {
        return m_recreation_registered;
    }

    void Set_Procedural(bool procedural) noexcept
    {
        m_procedural = procedural;
    }

    bool Is_Procedural() const noexcept
    {
        return m_procedural;
    }

    void Set_Initialized(bool initialized) noexcept
    {
        m_initialized = initialized;
    }

    bool Is_Initialized() const noexcept
    {
        return m_initialized;
    }

    bool Is_Resident() const noexcept
    {
        return m_resource != nullptr;
    }

    // Takes ownership of resource. A null resource is the canonical released
    // state and also clears initialization, so a later Ensure can load or
    // recreate the texture again.
    void Set_Resource(TextureResource* resource) noexcept
    {
        Set_Resource_At(resource, Now());
    }

    // Publish a completed load or an intermediate resource. When initialized
    // is false the prior initialization state is retained for a non-null
    // replacement, matching the old Apply_New_Surface contract.
    void Publish(TextureResource* resource, bool initialized,
        bool disable_auto_invalidation = false) noexcept
    {
        Set_Resource(resource);
        if (resource && initialized)
            m_initialized = true;
        else if (!resource)
            m_initialized = false;
        if (disable_auto_invalidation)
            m_inactivation_time = 0;
    }

    TextureResource* Resource() const noexcept
    {
        Touch();
        return m_resource;
    }

    RHITextureHandle Handle() const noexcept
    {
        Touch();
        return m_graphics_texture;
    }

    // Starts the owner-supplied load request, applying the cache extension
    // rule before the callback. This is also the operation exposed to callers
    // that historically called Init directly.
    void Initialize_If_Needed() noexcept
    {
        if (m_initialized)
            return;

        const auto now = Now();
        Prepare_Initialization(now);
        if (m_initialize)
            m_initialize();
        Touch(now);
    }

    // Reproduces the original order: touch, request a file load only when no
    // request is pending, return resident resources immediately, wait for a
    // pending load, then ask procedural owners to recreate their resource.
    bool Ensure() noexcept
    {
        return Ensure_At(Now());
    }

    bool Ensure_At(std::uint32_t now) noexcept
    {
        Touch(now);
        if (!m_procedural && !m_initialized && !Pending())
            Initialize_At(now);

        if (m_resource)
            return true;

        if (Pending())
            return false;

        if (m_procedural && m_recreate) {
            if (m_recreate())
                m_initialized = true;
        }

        Touch(now);
        return m_resource != nullptr;
    }

    bool Invalidate() noexcept
    {
        return Invalidate_At(Now());
    }

    void Set_Inactivation_Time(std::uint32_t milliseconds) noexcept
    {
        m_inactivation_time = milliseconds;
    }

    std::uint32_t Inactivation_Time() const noexcept
    {
        return m_inactivation_time;
    }

    std::uint32_t Extended_Inactivation_Time() const noexcept
    {
        return m_extended_inactivation_time;
    }

    std::uint32_t Last_Accessed() const noexcept
    {
        return m_last_accessed;
    }

    std::uint32_t Last_Inactivation_Sync_Time() const noexcept
    {
        return m_last_inactivation_sync_time;
    }

    // Applies strict age > threshold semantics and unsigned clock wrapping.
    // The timestamp is recorded even when Invalidate declines because the
    // resource is procedural or currently pending, as the legacy cache did.
    bool Evict_If_Old(std::uint32_t now, std::uint32_t override_time = 0) noexcept
    {
        if (!m_initialized || m_inactivation_time == 0)
            return false;

        const auto age = now - m_last_accessed;
        const auto threshold = override_time != 0
            ? override_time : m_inactivation_time + m_extended_inactivation_time;
        if (age <= threshold)
            return false;

        Invalidate_At(now);
        m_last_inactivation_sync_time = now;
        return true;
    }

    bool Pending() const noexcept
    {
        return m_load_source && Get_Resource_Load_Queue().Pending(m_load_source);
    }

private:
    // Keep a caller-supplied cache timestamp when replacing a generation from
    // an explicit eviction pass.
    void Set_Resource_At(TextureResource* resource, std::uint32_t now) noexcept
    {
        Touch(now);
        if (m_resource == resource) {
            m_graphics_texture = resource ? resource->Handle() : RHITextureHandle{};
            if (!resource)
                m_initialized = false;
            return;
        }

        // Clear the borrowed handle before releasing the owner. This prevents
        // callers from observing a generation that has just been destroyed.
        m_graphics_texture = {};
        Release_Texture_Resource(m_resource);
        m_resource = resource;
        m_graphics_texture = resource ? resource->Handle() : RHITextureHandle{};
        if (!resource)
            m_initialized = false;
    }

    bool Invalidate_At(std::uint32_t now) noexcept
    {
        if (Pending() || m_procedural)
            return false;

        Set_Resource_At(nullptr, now);
        return true;
    }

    std::uint32_t Now() const noexcept
    {
        return m_clock ? m_clock() : 0;
    }

    void Touch() const noexcept
    {
        m_last_accessed = Now();
    }

    void Touch(std::uint32_t now) const noexcept
    {
        m_last_accessed = now;
    }

    void Prepare_Initialization(std::uint32_t now) noexcept
    {
        if (m_inactivation_time && m_last_inactivation_sync_time) {
            if (now - m_last_inactivation_sync_time < m_inactivation_time)
                m_extended_inactivation_time = 3 * m_inactivation_time;
            m_last_inactivation_sync_time = 0;
        }
    }

    void Initialize_At(std::uint32_t now) noexcept
    {
        if (m_initialized)
            return;

        Prepare_Initialization(now);
        if (m_initialize)
            m_initialize();
        Touch(now);
    }

    TextureResidencyClock m_clock;
    std::shared_ptr<const ResourceLoadSource> m_load_source;
    Initialize m_initialize;
    Recreate m_recreate;
    ResourceRecreationRegistration m_recreation_registration;
    TextureResource* m_resource = nullptr;
    RHITextureHandle m_graphics_texture{};
    std::uint32_t m_inactivation_time = 0;
    std::uint32_t m_extended_inactivation_time = 0;
    std::uint32_t m_last_inactivation_sync_time = 0;
    mutable std::uint32_t m_last_accessed = 0;
    bool m_initialized = false;
    bool m_procedural = false;
    bool m_recreation_registered = false;
};

}
