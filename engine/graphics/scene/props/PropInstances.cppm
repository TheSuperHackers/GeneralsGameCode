module;
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Props.Instances;
export import Graphics.Scene.Props.SkinPalettes;
import Graphics.RHI;
import Graphics.Resources.Handles.ResourceHandle;
import Graphics.Resources.Pools.ResourcePool;
import Graphics.Scene.Props.Constants;

namespace Graphics {
export struct PropInstanceTag;
export using PropInstanceHandle = ResourceHandle<PropInstanceTag>;

// This layout is shared with the prop shader's structured buffer.
export struct PropInstanceData final {
    std::array<float,16> world{};
    std::array<std::byte,464> lighting{};
    std::array<std::uint32_t,4> skin{};
};
static_assert(sizeof(PropInstanceData) == 544);

// Handles identify immutable submission snapshots. An owner may change its
// record in place only while no deferred consumer retains that generation.
export class PropInstances final {
public:
    PropInstanceHandle Update(PropInstanceHandle handle, const PropParameters& parameters,
        PropSkinPaletteHandle skin = {}) {
        PropInstanceData value;
        value.world = parameters.world;
        value.skin = m_palettes.Address(skin);
        assert(!skin.Is_Valid() || value.skin[1] != 0);
        std::memcpy(value.lighting.data(), &parameters.scene_ambient, value.lighting.size());
        auto* entry = m_entries.Resolve(handle);
        if (entry && entry->skin == skin && std::memcmp(&m_values[handle.Get_Index()], &value, sizeof(value)) == 0) return handle;
        if (skin.Is_Valid()) m_palettes.Retain(skin);
        if (!entry || entry->references != 1) {
            Release(handle);
            handle = m_entries.Create(Entry{});
            if (m_values.size() <= handle.Get_Index()) m_values.resize(handle.Get_Index()+1);
            entry = m_entries.Resolve(handle);
        } else m_palettes.Release(entry->skin);
        entry->skin = skin;
        m_values[handle.Get_Index()] = value;
        m_first_dirty = (std::min)(m_first_dirty, std::size_t(handle.Get_Index()));
        m_dirty_end = (std::max)(m_dirty_end, std::size_t(handle.Get_Index()+1));
        return handle;
    }
    bool Retain(PropInstanceHandle handle) noexcept {
        auto* entry = m_entries.Resolve(handle);
        if (!entry) return false;
        assert(entry->references != (std::numeric_limits<std::size_t>::max)());
        ++entry->references;
        return true;
    }
    void Release(PropInstanceHandle handle) noexcept {
        auto* entry = m_entries.Resolve(handle);
        if (entry && --entry->references == 0) {
            m_palettes.Release(entry->skin);
            m_entries.Destroy(handle);
        }
    }
    const PropInstanceData* Resolve(PropInstanceHandle handle) const noexcept {
        return m_entries.Resolve(handle) ? &m_values[handle.Get_Index()] : nullptr;
    }
    // Draw queues retain and validate generations before exposing GPU indices.
    const PropInstanceData& At_Index(std::uint32_t index) const noexcept {
        assert(index<m_values.size());
        return m_values[index];
    }
    PropSkinPalettes& Palettes() noexcept { return m_palettes; }
    std::span<const PropBoneTransform> Pose(PropInstanceHandle handle) const noexcept {
        const auto* entry = m_entries.Resolve(handle);
        return entry ? m_palettes.Resolve(entry->skin) : std::span<const PropBoneTransform>{};
    }
    bool Prepare(Device& device) {
        if (!m_palettes.Prepare(device)) return false;
        if (m_values.empty()) return false;
        if (m_values.size() > m_capacity) {
            const auto capacity = std::bit_ceil(m_values.size());
            if (capacity > (std::numeric_limits<std::uint32_t>::max)()/sizeof(PropInstanceData)) return false;
            const auto buffer = device.Create_Buffer(
                {static_cast<std::uint32_t>(capacity*sizeof(PropInstanceData)), RHIBufferUsage::Storage,
                    sizeof(PropInstanceData)});
            if (!buffer.Is_Valid()) return false;
            if (!device.Update_Buffer(buffer,0,std::as_bytes(std::span(m_values)))) {
                device.Destroy_Buffer(buffer);
                return false;
            }
            m_uploaded_bytes += m_values.size()*sizeof(PropInstanceData);
            if (m_buffer.Is_Valid()) device.Destroy_Buffer(m_buffer);
            m_buffer = buffer;
            m_capacity = capacity;
        } else if (m_first_dirty < m_dirty_end) {
            const auto bytes = std::as_bytes(std::span(m_values).subspan(m_first_dirty,m_dirty_end-m_first_dirty));
            if (!device.Update_Buffer(m_buffer,static_cast<std::uint32_t>(m_first_dirty*sizeof(PropInstanceData)),bytes)) return false;
            m_uploaded_bytes += bytes.size();
        }
        m_first_dirty = (std::numeric_limits<std::size_t>::max)();
        m_dirty_end = 0;
        return true;
    }
    void Shutdown(Device& device) noexcept {
        m_palettes.Shutdown(device);
        if (m_buffer.Is_Valid()) device.Destroy_Buffer(m_buffer);
        m_buffer = {};
        m_capacity = 0;
    }
    RHIBufferHandle Buffer() const noexcept { return m_buffer; }
    std::uint64_t Uploaded_Bytes() const noexcept { return m_uploaded_bytes; }
private:
    struct Entry { std::size_t references = 1; PropSkinPaletteHandle skin{}; };
    PropSkinPalettes m_palettes;
    ResourcePool<Entry,PropInstanceHandle> m_entries;
    std::vector<PropInstanceData> m_values;
    RHIBufferHandle m_buffer{};
    std::size_t m_capacity = 0;
    std::size_t m_first_dirty = (std::numeric_limits<std::size_t>::max)();
    std::size_t m_dirty_end = 0;
    std::uint64_t m_uploaded_bytes = 0;
};

export class PropInstanceOwner final {
public:
    PropInstanceOwner() = default;
    PropInstanceOwner(const PropInstanceOwner&) = delete;
    PropInstanceOwner& operator=(const PropInstanceOwner&) = delete;
    ~PropInstanceOwner() { Reset(); }
    void Reset() noexcept {
        if (m_instances) m_instances->Release(m_handle);
        m_instances=nullptr; m_handle={};
    }
    PropInstanceHandle Update(PropInstances& instances, const PropParameters& parameters,
        PropSkinPaletteHandle skin = {}) {
        if (m_instances != &instances) {
            if (m_instances) m_instances->Release(m_handle);
            m_instances = &instances;
            m_handle = {};
        }
        m_handle = instances.Update(m_handle,parameters,skin);
        return m_handle;
    }
private:
    PropInstances* m_instances = nullptr;
    PropInstanceHandle m_handle{};
};
}
