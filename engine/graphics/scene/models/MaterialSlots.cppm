module;

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

export module Graphics.Scene.Models.MaterialSlots;
import Graphics.Scene.Models.SourceRevision;

namespace Graphics {

// A material slot collection owns the storage for caller-selected resource
// owners.  Copies share the storage so alternate descriptions can share a
// complete slot collection; Clone makes an independent slot vector while
// retaining each owner through its normal copy operation.
export template<class Owner>
class MaterialSlots final
{
    struct Storage final
    {
        explicit Storage(std::size_t count) : values(count) {}

        std::vector<Owner> values;
        SourceRevision revision;
    };

public:
    MaterialSlots() noexcept = default;
    MaterialSlots(const MaterialSlots&) noexcept = default;
    MaterialSlots(MaterialSlots&&) noexcept = default;
    MaterialSlots& operator=(const MaterialSlots&) noexcept = default;
    MaterialSlots& operator=(MaterialSlots&&) noexcept = default;
    ~MaterialSlots() = default;

    bool Is_Allocated() const noexcept
    {
        return static_cast<bool>(m_storage);
    }

    std::size_t Count() const noexcept
    {
        return m_storage ? m_storage->values.size() : 0;
    }

    // Allocation is separate from the default state because an allocated
    // zero-length array is meaningful to material descriptions.
    void Allocate(std::size_t count)
    {
        if (!m_storage) {
            m_storage = std::make_shared<Storage>(count);
        } else {
            m_storage->revision.Invalidate();
            m_storage->values.resize(count);
        }
    }

    void Reset() noexcept
    {
        m_storage.reset();
    }

    MaterialSlots Clone() const
    {
        MaterialSlots result;
        if (m_storage) {
            result.m_storage = std::make_shared<Storage>(*m_storage);
            result.m_storage->revision.Reset();
        }
        return result;
    }

    // Get copies the caller-supplied owner.  This gives callers a retained
    // resource without imposing a reference-counting protocol on Owner.
    Owner Get(std::size_t index) const
    {
        if (!m_storage || index >= m_storage->values.size()) {
            return Owner{};
        }
        return m_storage->values[index];
    }

    // Peek borrows the owner slot.  The owner itself controls how its resource
    // is inspected and retained by the caller.
    Owner* Peek(std::size_t index) noexcept
    {
        if (!m_storage || index >= m_storage->values.size()) {
            return nullptr;
        }
        m_storage->revision.Expose_Writable();
        return &m_storage->values[index];
    }

    const Owner* Peek(std::size_t index) const noexcept
    {
        if (!m_storage || index >= m_storage->values.size()) {
            return nullptr;
        }
        return &m_storage->values[index];
    }

    void Set(std::size_t index, const Owner& owner)
    {
        assert(m_storage && index < m_storage->values.size());
        if (!m_storage || index >= m_storage->values.size()) {
            return;
        }
        m_storage->revision.Invalidate();
        m_storage->values[index] = owner;
    }

    std::uint64_t Revision() const noexcept
    {
        return m_storage ? m_storage->revision.Token() : 0;
    }

private:
    std::shared_ptr<Storage> m_storage;
};

}
