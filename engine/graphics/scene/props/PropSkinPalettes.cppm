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
export module Graphics.Scene.Props.SkinPalettes;
import Graphics.RHI;
import Graphics.Resources.Handles.ResourceHandle;
import Graphics.Resources.Pools.ResourcePool;

namespace Graphics {
export struct PropSkinPaletteTag;
export using PropSkinPaletteHandle = ResourceHandle<PropSkinPaletteTag>;
export using PropBoneTransform = std::array<float,12>;

export std::array<float,3> Transform_Prop_Skin_Position(const std::array<float,3>& position,
    const PropBoneTransform& transform) noexcept
{
    std::array<float,3> result;
    for (unsigned axis=0; axis<3; ++axis) {
        const auto row=axis*4;
        result[axis]=transform[row]*position[0]+transform[row+1]*position[1]
            +transform[row+2]*position[2]+transform[row+3];
    }
    return result;
}

// Palette ranges are stable while retained. Released power-of-two ranges are
// reused, so changing a queued pose needs a snapshot, not a new GPU allocation.
export class PropSkinPalettes final {
public:
    template<class ReadTransform>
    PropSkinPaletteHandle Update(PropSkinPaletteHandle handle, std::size_t count,
        const ReadTransform& read_transform)
    {
        assert(count != 0 && count <= 65536);
        auto* entry=m_entries.Resolve(handle);
        bool same=entry && entry->count==count;
        for (std::size_t bone=0; same && bone<count; ++bone) {
            const auto& matrix=read_transform(bone);
            same=std::memcmp(m_values[entry->first+bone].data(),matrix.data(),sizeof(PropBoneTransform))==0;
        }
        if (same) return handle;
        if (!entry || entry->references!=1 || count>entry->capacity) {
            Release(handle);
            const auto capacity=std::bit_ceil(count);
            auto& free=m_free_ranges[std::countr_zero(capacity)];
            std::size_t first;
            if (free.empty()) {
                free.reserve(std::bit_ceil(++m_range_counts[std::countr_zero(capacity)]));
                first=m_values.size();
                m_values.resize(first+capacity);
            } else { first=free.back(); free.pop_back(); }
            handle=m_entries.Create(Entry{first,count,capacity});
            entry=m_entries.Resolve(handle);
        }
        entry->count=count;
        for (std::size_t bone=0; bone<count; ++bone) {
            const auto& matrix=read_transform(bone);
            std::memcpy(m_values[entry->first+bone].data(),matrix.data(),sizeof(PropBoneTransform));
        }
        m_first_dirty=(std::min)(m_first_dirty,entry->first);
        m_dirty_end=(std::max)(m_dirty_end,entry->first+count);
        return handle;
    }
    bool Retain(PropSkinPaletteHandle handle) noexcept {
        auto* entry=m_entries.Resolve(handle);
        if (!entry) return false;
        assert(entry->references!=(std::numeric_limits<std::size_t>::max)());
        ++entry->references;
        return true;
    }
    void Release(PropSkinPaletteHandle handle) noexcept {
        auto* entry=m_entries.Resolve(handle);
        if (!entry || --entry->references!=0) return;
        m_free_ranges[std::countr_zero(entry->capacity)].push_back(entry->first);
        m_entries.Destroy(handle);
    }
    std::span<const PropBoneTransform> Resolve(PropSkinPaletteHandle handle) const noexcept {
        const auto* entry=m_entries.Resolve(handle);
        return entry ? std::span<const PropBoneTransform>(m_values).subspan(entry->first,entry->count)
            : std::span<const PropBoneTransform>{};
    }
    std::array<std::uint32_t,4> Address(PropSkinPaletteHandle handle) const noexcept {
        const auto* entry=m_entries.Resolve(handle);
        if (!entry) return {};
        assert(entry->first+entry->count <= (std::numeric_limits<std::uint32_t>::max)()/sizeof(PropBoneTransform));
        return {static_cast<std::uint32_t>(entry->first),static_cast<std::uint32_t>(entry->count),0,0};
    }
    bool Prepare(Device& device) {
        if (m_values.size()>m_capacity) {
            const auto capacity=std::bit_ceil(m_values.size());
            if (capacity>(std::numeric_limits<std::uint32_t>::max)()/sizeof(PropBoneTransform)) return false;
            const auto buffer=device.Create_Buffer({static_cast<std::uint32_t>(capacity*sizeof(PropBoneTransform)),
                RHIBufferUsage::Storage,sizeof(PropBoneTransform)});
            if (!buffer.Is_Valid()) return false;
            if (!device.Update_Buffer(buffer,0,std::as_bytes(std::span(m_values)))) {
                device.Destroy_Buffer(buffer); return false;
            }
            m_uploaded_bytes+=m_values.size()*sizeof(PropBoneTransform);
            if (m_buffer.Is_Valid()) device.Destroy_Buffer(m_buffer);
            m_buffer=buffer; m_capacity=capacity;
        } else if (m_first_dirty<m_dirty_end) {
            const auto bytes=std::as_bytes(std::span(m_values).subspan(m_first_dirty,m_dirty_end-m_first_dirty));
            if (!device.Update_Buffer(m_buffer,static_cast<std::uint32_t>(m_first_dirty*sizeof(PropBoneTransform)),bytes)) return false;
            m_uploaded_bytes+=bytes.size();
        }
        m_first_dirty=(std::numeric_limits<std::size_t>::max)(); m_dirty_end=0;
        return true;
    }
    void Shutdown(Device& device) noexcept {
        if (m_buffer.Is_Valid()) device.Destroy_Buffer(m_buffer);
        m_buffer={}; m_capacity=0;
    }
    RHIBufferHandle Buffer() const noexcept { return m_buffer; }
    std::uint64_t Uploaded_Bytes() const noexcept { return m_uploaded_bytes; }
private:
    struct Entry {
        std::size_t first, count, capacity;
        std::size_t references=1;
    };
    ResourcePool<Entry,PropSkinPaletteHandle> m_entries;
    // A valid identity record also supplies the binding for unskinned draws.
    std::vector<PropBoneTransform> m_values{{1,0,0,0,0,1,0,0,0,0,1,0}};
    std::array<std::vector<std::size_t>,17> m_free_ranges;
    std::array<std::size_t,17> m_range_counts{};
    RHIBufferHandle m_buffer{};
    std::size_t m_capacity=0;
    std::size_t m_first_dirty=0, m_dirty_end=1;
    std::uint64_t m_uploaded_bytes=0;
};

// Keep a borrowed pose alive across callbacks that may update its owner.
export class PropSkinLease final {
public:
    PropSkinLease(PropSkinPalettes& palettes,PropSkinPaletteHandle handle) noexcept
        : m_palettes(palettes),m_handle(handle) {
        if (handle.Is_Valid()) {
            const bool retained=m_palettes.Retain(handle);
            assert(retained);
        }
    }
    ~PropSkinLease() { m_palettes.Release(m_handle); }
    PropSkinLease(const PropSkinLease&)=delete;
    PropSkinLease& operator=(const PropSkinLease&)=delete;
private:
    PropSkinPalettes& m_palettes;
    PropSkinPaletteHandle m_handle;
};

export class PropSkinOwner final {
public:
    PropSkinOwner()=default;
    PropSkinOwner(const PropSkinOwner&)=delete;
    PropSkinOwner& operator=(const PropSkinOwner&)=delete;
    ~PropSkinOwner() { Reset(); }
    void Reset() noexcept {
        if (m_palettes) m_palettes->Release(m_handle);
        m_palettes=nullptr; m_handle={};
    }
    template<class ReadTransform>
    PropSkinPaletteHandle Update(PropSkinPalettes& palettes,std::size_t count,const ReadTransform& read_transform) {
        if (m_palettes!=&palettes) {
            if (m_palettes) m_palettes->Release(m_handle);
            m_palettes=&palettes; m_handle={};
        }
        m_handle=palettes.Update(m_handle,count,read_transform);
        return m_handle;
    }
private:
    PropSkinPalettes* m_palettes=nullptr;
    PropSkinPaletteHandle m_handle{};
};
}
