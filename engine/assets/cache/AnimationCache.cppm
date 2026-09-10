module;
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
export module Assets.Cache.Animations;
export import Assets.Animations;
export import Assets.Handles;
import Assets.Identity;

namespace Assets {
// Consecutive clips interpolate their last frame toward frame zero. Keyed clips
// retain the endpoints and step policy encoded in their channels.
export enum class AnimationSampling { Consecutive, Keyed, Pose };
export struct AnimationClip final {
    std::string name;
    std::string skeleton_name;
    std::uint32_t frame_count=0;
    float frame_rate=0;
    std::uint32_t bone_count=0;
    AnimationSampling sampling=AnimationSampling::Consecutive;
    AnimationAsset channels;
    ModelPoseAnimationDesc pose;
    std::vector<AnimationAssetHandle> sources;
};

// Publication, retention, and eviction run on the asset owner's thread.
// A named entry belongs to the cache; external users and pose dependencies hold
// explicit references. Clearing names never invalidates a retained clip.
export class AnimationCache final {
public:
    AnimationCache()=default;
    AnimationCache(const AnimationCache&)=delete;
    AnimationCache& operator=(const AnimationCache&)=delete;

    AnimationAssetHandle Publish(ModelAnimationDesc description,std::uint32_t bones,
        AnimationSampling sampling,std::string& error) {
        if(sampling!=AnimationSampling::Consecutive && sampling!=AnimationSampling::Keyed) {
            error="invalid channel sampling policy";return {};
        }
        auto clip=std::make_unique<AnimationClip>();
        clip->name=description.skeleton_name+"."+description.name;
        clip->skeleton_name=description.skeleton_name;
        clip->frame_count=description.frame_count;clip->frame_rate=description.frame_rate;
        clip->bone_count=bones;clip->sampling=sampling;
        if(!clip->channels.Initialize(std::move(description),bones,error))return {};
        return Insert(std::move(clip),error);
    }

    AnimationAssetHandle Publish_Pose(ModelPoseAnimationDesc description,std::uint32_t bones,
        std::vector<AnimationAssetHandle> sources,std::string& error) {
        if(!Validate_Pose_Animation(description,error))return {};
        if(!bones || bones>std::uint32_t(std::numeric_limits<int>::max())
            || description.frame_count>std::uint32_t(std::numeric_limits<int>::max())
            || sources.size()!=description.channels.size()) {
            error="invalid pose animation dimensions";return {};
        }
        if(description.bone_channels.empty())description.bone_channels.assign(bones,0);
        if(description.bone_channels.size()!=bones) {
            error="pose mapping does not match skeleton";return {};
        }
        for(std::size_t i=0;i<sources.size();++i) {
            const auto* source=Resolve(sources[i]);
            if(!source || Canonicalize_Asset_Name(source->name)!=Canonicalize_Asset_Name(description.channels[i].animation_name)) {
                error="unresolved pose animation source";return {};
            }
        }
        auto clip=std::make_unique<AnimationClip>();
        clip->name=description.skeleton_name+"."+description.name;
        clip->skeleton_name=description.skeleton_name;
        clip->frame_count=description.frame_count;clip->frame_rate=description.frame_rate;
        clip->bone_count=bones;clip->sampling=AnimationSampling::Pose;
        clip->pose=std::move(description);clip->sources=std::move(sources);
        // Dependencies can only name previously published clips, so this graph
        // is acyclic. Acquire them before publication and unwind on failure.
        std::size_t acquired=0;
        for(const auto source:clip->sources) {
            if(!Retain(source)) {
                for(std::size_t i=0;i<acquired;++i)Release(clip->sources[i]);
                error="pose source reference limit reached";return {};
            }
            ++acquired;
        }
        const auto dependencies=clip->sources;
        const auto handle=Insert(std::move(clip),error);
        if(!handle)for(const auto source:dependencies)Release(source);
        return handle;
    }

    AnimationAssetHandle Find(std::string_view name) const {
        const auto found=m_names.find(Canonicalize_Asset_Name(name));
        return found==m_names.end() ? AnimationAssetHandle{} : found->second;
    }
    AnimationAssetHandle Acquire(std::string_view name) {
        const auto handle=Find(name);return Retain(handle) ? handle : AnimationAssetHandle{};
    }
    const AnimationClip* Resolve(AnimationAssetHandle handle) const noexcept {
        const auto* entry=Lookup(handle);return entry ? entry->clip.get() : nullptr;
    }
    bool Retain(AnimationAssetHandle handle) noexcept {
        auto* entry=Lookup(handle);
        if(!entry || entry->references==std::numeric_limits<std::uint64_t>::max())return false;
        ++entry->references;return true;
    }
    bool Release(AnimationAssetHandle handle) {
        auto* entry=Lookup(handle);
        if(!entry || !entry->references)return false;
        --entry->references;
        if(!entry->references && !entry->named)Destroy(handle.Get_Index());
        return true;
    }
    std::uint64_t Reference_Count(AnimationAssetHandle handle) const noexcept {
        const auto* entry=Lookup(handle);return entry ? entry->references : 0;
    }
    template<class Predicate>
    void Remove_Unused_If(Predicate&& remove) {
        for(std::uint32_t i=0;i<m_entries.size();++i) {
            const auto& entry=m_entries[i];
            if(entry.clip && entry.named && !entry.references && remove(*entry.clip))Destroy(i);
        }
    }
    template<class Visitor>
    void Visit_Named(Visitor&& visit) const {
        for(const auto& entry:m_entries)if(entry.clip && entry.named)visit(*entry.clip);
    }
    void Clear_Named() {
        m_names.clear();
        // Detach every name before releasing dependencies. A retained pose can
        // then keep any source alive independently of insertion order.
        for(auto& entry:m_entries)entry.named=false;
        for(std::uint32_t i=0;i<m_entries.size();++i)
            if(m_entries[i].clip && !m_entries[i].references)Destroy(i);
    }
    void Clear() { Clear_Named();Reset_Missing(); }
    void Register_Missing(std::string_view name) { m_missing.insert(Canonicalize_Asset_Name(name)); }
    bool Is_Missing(std::string_view name) const { return m_missing.contains(Canonicalize_Asset_Name(name)); }
    void Reset_Missing() { m_missing.clear(); }
    std::size_t Size() const noexcept { return m_names.size(); }

private:
    struct Entry {
        std::unique_ptr<const AnimationClip> clip;
        std::string identity;
        std::uint64_t references=0;
        std::uint32_t generation=1;
        bool named=false;
    };
    const Entry* Lookup(AnimationAssetHandle handle) const noexcept {
        if(!handle || handle.Get_Index()>=m_entries.size())return nullptr;
        const auto& entry=m_entries[handle.Get_Index()];
        return entry.clip && entry.generation==handle.Get_Generation() ? &entry : nullptr;
    }
    Entry* Lookup(AnimationAssetHandle handle) noexcept {
        return const_cast<Entry*>(std::as_const(*this).Lookup(handle));
    }
    AnimationAssetHandle Insert(std::unique_ptr<AnimationClip> clip,std::string& error) {
        auto identity=Canonicalize_Asset_Name(clip->name);
        if(clip->skeleton_name.empty() || m_names.contains(identity)) {
            error="empty or duplicate animation identity";return {};
        }
        std::uint32_t index;
        if(m_free.empty()) {
            if(m_entries.size()>=std::numeric_limits<std::uint32_t>::max()) {
                error="animation handle index space exhausted";return {};
            }
            index=static_cast<std::uint32_t>(m_entries.size());m_entries.emplace_back();
        } else { index=m_free.back();m_free.pop_back(); }
        auto& entry=m_entries[index];
        entry.identity=std::move(identity);entry.clip=std::move(clip);entry.named=true;
        const AnimationAssetHandle handle(index,entry.generation);
        m_names.emplace(entry.identity,handle);m_missing.erase(entry.identity);
        error.clear();return handle;
    }
    void Destroy(std::uint32_t index) {
        auto& entry=m_entries[index];
        auto clip=std::move(entry.clip);
        if(entry.named)m_names.erase(entry.identity);
        entry.named=false;entry.identity.clear();
        if(entry.generation==std::numeric_limits<std::uint32_t>::max())entry.generation=0;
        else { ++entry.generation;m_free.push_back(index); }
        for(const auto source:clip->sources)Release(source);
    }
    std::vector<Entry> m_entries;
    std::vector<std::uint32_t> m_free;
    std::unordered_map<std::string,AnimationAssetHandle> m_names;
    std::unordered_set<std::string> m_missing;
};
export AnimationCache& Get_Animation_Cache() { static AnimationCache cache;return cache; }
export void Release_Animation(AnimationAssetHandle& handle) {
    Get_Animation_Cache().Release(handle);handle={};
}
}
