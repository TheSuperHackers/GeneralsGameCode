module;
#include <cstdint>
#include <functional>
#include <utility>
export module Graphics.Resources.Recreation;

namespace Graphics
{
export class ResourceRecreationRegistry;

// Embedded in the resource owner, and reset before its captured state dies.
// All registration and dispatch operations run on the device thread.
export class ResourceRecreationRegistration final
{
public:
    ResourceRecreationRegistration() = default;
    ResourceRecreationRegistration(const ResourceRecreationRegistration&) = delete;
    ResourceRecreationRegistration& operator=(const ResourceRecreationRegistration&) = delete;
    ResourceRecreationRegistration(ResourceRecreationRegistration&& other) noexcept { Move(other); }
    ResourceRecreationRegistration& operator=(ResourceRecreationRegistration&& other) noexcept
    {
        if (this!=&other) { Reset(); Move(other); }
        return *this;
    }
    ~ResourceRecreationRegistration() { Reset(); }
    void Reset() noexcept;
private:
    friend class ResourceRecreationRegistry;
    void Move(ResourceRecreationRegistration& other) noexcept;
    ResourceRecreationRegistry* m_registry=nullptr;
    ResourceRecreationRegistration* m_previous=nullptr;
    ResourceRecreationRegistration* m_next=nullptr;
    std::function<void()> m_release,m_recreate;
    std::uint64_t m_sequence=0;
};

export class ResourceRecreationRegistry final
{
public:
    ResourceRecreationRegistry() = default;
    ResourceRecreationRegistry(const ResourceRecreationRegistry&) = delete;
    ResourceRecreationRegistry& operator=(const ResourceRecreationRegistry&) = delete;
    ~ResourceRecreationRegistry() { while (m_first) m_first->Reset(); }

    ResourceRecreationRegistration Register(std::function<void()> release,std::function<void()> recreate)
    {
        ResourceRecreationRegistration registration;
        registration.m_release=std::move(release);
        registration.m_recreate=std::move(recreate);
        registration.m_registry=this;
        registration.m_previous=m_last;
        registration.m_sequence=++m_sequence;
        if (m_last) m_last->m_next=&registration;
        else m_first=&registration;
        m_last=&registration;
        return registration;
    }
    void Release() { Dispatch(false); }
    void Recreate() { Dispatch(true); }

private:
    friend class ResourceRecreationRegistration;
    struct Cursor { ResourceRecreationRegistration* next; Cursor* outer; };
    void Redirect(ResourceRecreationRegistration* from,ResourceRecreationRegistration* to) noexcept
    {
        for (auto* cursor=m_cursor;cursor;cursor=cursor->outer)
            if (cursor->next==from) cursor->next=to;
    }
    void Dispatch(bool recreate)
    {
        const auto sequence=m_sequence;
        Cursor cursor{m_first,m_cursor};
        m_cursor=&cursor;
        struct Restore { Cursor*& current; Cursor* outer; ~Restore() { current=outer; } } restore{m_cursor,cursor.outer};
        while (cursor.next && cursor.next->m_sequence<=sequence) {
            auto* registration=cursor.next;
            cursor.next=registration->m_next;
            // Callbacks may unregister themselves or another owner. Copy only
            // the current invocation; removed future owners are never called.
            const auto callback=recreate ? registration->m_recreate : registration->m_release;
            if (callback) callback();
        }
    }
    ResourceRecreationRegistration* m_first=nullptr;
    ResourceRecreationRegistration* m_last=nullptr;
    Cursor* m_cursor=nullptr;
    std::uint64_t m_sequence=0;
};

void ResourceRecreationRegistration::Reset() noexcept
{
    if (m_registry) {
        m_registry->Redirect(this,m_next);
        if (m_previous) m_previous->m_next=m_next;
        else m_registry->m_first=m_next;
        if (m_next) m_next->m_previous=m_previous;
        else m_registry->m_last=m_previous;
        m_registry=nullptr;
        m_previous=m_next=nullptr;
    }
    m_release={}; m_recreate={};
}

void ResourceRecreationRegistration::Move(ResourceRecreationRegistration& other) noexcept
{
    m_registry=std::exchange(other.m_registry,nullptr);
    m_previous=std::exchange(other.m_previous,nullptr);
    m_next=std::exchange(other.m_next,nullptr);
    m_sequence=other.m_sequence;
    m_release=std::move(other.m_release); m_recreate=std::move(other.m_recreate);
    if (!m_registry) return;
    m_registry->Redirect(&other,this);
    if (m_previous) m_previous->m_next=this; else m_registry->m_first=this;
    if (m_next) m_next->m_previous=this; else m_registry->m_last=this;
}

export ResourceRecreationRegistry& Get_Resource_Recreation_Registry()
{
    static ResourceRecreationRegistry registry;
    return registry;
}
}
