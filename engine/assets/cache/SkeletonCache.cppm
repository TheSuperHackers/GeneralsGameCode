module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module Assets.Cache.Skeletons;
export import Assets.Handles;
export import Assets.ModelRig;
import Assets.Identity;

namespace Assets {

// Prepared skeletons are published and released on the asset owner's thread.
// Source IO and decoding remain with adapters; poses belong to consumers.
export class SkeletonCache final {
public:
    SkeletonCache() = default;
    SkeletonCache(const SkeletonCache&) = delete;
    SkeletonCache& operator=(const SkeletonCache&) = delete;

    SkeletonAssetHandle Publish(ModelRigDesc description, std::string& error) {
        if(description.bones.empty() || !description.animations.empty() || !description.attachments.empty()) {
            error="skeleton publication requires bones without clips or model attachments";
            return {};
        }
        if(!Validate_Model_Rig(description,error)) return {};
        auto identity=Canonicalize_Asset_Name(description.skeleton_name);
        if(identity.empty() || m_names.contains(identity)) {
            error="empty or duplicate skeleton identity";
            return {};
        }
        auto asset=std::make_unique<const ModelRigDesc>(std::move(description));
        std::uint32_t index;
        if(m_free.empty()) {
            if(m_entries.size()>=std::numeric_limits<std::uint32_t>::max()) {
                error="skeleton handle index space exhausted";
                return {};
            }
            index=static_cast<std::uint32_t>(m_entries.size());
            m_entries.emplace_back();
        } else {
            index=m_free.back();
            m_free.pop_back();
        }
        auto& entry=m_entries[index];
        const SkeletonAssetHandle handle(index,entry.generation);
        entry.identity=std::move(identity);
        entry.asset=std::move(asset);
        m_names.emplace(entry.identity,handle);
        error.clear();
        return handle;
    }

    SkeletonAssetHandle Find(std::string_view name) const {
        const auto found=m_names.find(Canonicalize_Asset_Name(name));
        return found==m_names.end() ? SkeletonAssetHandle{} : found->second;
    }

    // Borrowed until removal or cache destruction. Inserting other skeletons
    // does not move the asset. Consumers retain handles and resolve before use.
    const ModelRigDesc* Resolve(SkeletonAssetHandle handle) const noexcept {
        if(!handle.Is_Valid() || handle.Get_Index()>=m_entries.size()) return nullptr;
        const auto& entry=m_entries[handle.Get_Index()];
        return entry.generation==handle.Get_Generation() ? entry.asset.get() : nullptr;
    }

    template<class Predicate>
    void Remove_If(Predicate&& remove) {
        for(std::uint32_t index=0;index<m_entries.size();++index) {
            auto& entry=m_entries[index];
            if(!entry.asset || !remove(*entry.asset)) continue;
            m_names.erase(entry.identity);
            entry.asset.reset();
            entry.identity.clear();
            // Retire a slot instead of allowing a wrapped generation to revive
            // a handle retained by a previous level.
            if(entry.generation==std::numeric_limits<std::uint32_t>::max()) {
                entry.generation=0;
            } else {
                ++entry.generation;
                m_free.push_back(index);
            }
        }
    }

    void Clear() { Remove_If([](const ModelRigDesc&) { return true; }); }
    std::size_t Size() const noexcept { return m_names.size(); }

private:
    struct Entry {
        std::unique_ptr<const ModelRigDesc> asset;
        std::string identity;
        std::uint32_t generation=1;
    };
    std::vector<Entry> m_entries;
    std::vector<std::uint32_t> m_free;
    std::unordered_map<std::string,SkeletonAssetHandle> m_names;
};
}
