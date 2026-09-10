module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <span>
#include <utility>
#include <vector>
export module Graphics.Scene.Models.Children;

namespace Graphics {

// Attachment storage owns caller-selected resource owners. Graphics controls
// ordered traversal and level selection; callbacks translate scene operations.
export template<class Owner>
class ModelChildren final {
public:
    struct Attachment {
        Owner model;
        int bone = 0;
    };
    struct RemovedAttachment {
        Attachment attachment;
        int level = -1;
    };

    ModelChildren() = default;
    ModelChildren(const ModelChildren&) = delete;
    ModelChildren& operator=(const ModelChildren&) = delete;

    void Initialize(std::size_t levels) {
        assert(Count() == 0);
        m_levels.clear();
        m_levels.resize(levels);
        m_current = 0;
    }

    int Level_Count() const noexcept { return static_cast<int>(m_levels.size()); }
    int Current_Level() const noexcept { return m_current; }
    std::span<const Attachment> Level(int level) const {
        assert(level >= 0 && level < Level_Count());
        return m_levels[level];
    }
    std::span<const Attachment> Additional() const noexcept { return m_additional; }

    void Add(int level, Owner model, int bone) {
        assert(level >= 0 && level < Level_Count());
        m_levels[level].push_back({std::move(model), bone});
    }
    void Add_Additional(Owner model, int bone) {
        m_additional.push_back({std::move(model), bone});
    }

    template<class Clone>
    void Clone_From(const ModelChildren& source, Clone&& clone) {
        assert(Count() == 0);
        Initialize(source.m_levels.size());
        for (int level = 0; level < Level_Count(); ++level) {
            m_levels[level].reserve(source.m_levels[level].size());
            for (const auto& attachment : source.m_levels[level])
                Add(level, clone(attachment.model), attachment.bone);
        }
        m_additional.reserve(source.m_additional.size());
        for (const auto& attachment : source.m_additional)
            Add_Additional(clone(attachment.model), attachment.bone);
    }

    template<class Visitor>
    void Visit_All(Visitor&& visit) const {
        for (int level = 0; level < Level_Count(); ++level)
            for (const auto& attachment : m_levels[level]) visit(attachment, level);
        for (const auto& attachment : m_additional) visit(attachment, -1);
    }

    template<class Visitor>
    void Visit_Level(int level, Visitor&& visit, bool additional = true) const {
        for (const auto& attachment : Level(level)) visit(attachment, level);
        if (additional)
            for (const auto& attachment : m_additional) visit(attachment, -1);
    }

    // Queries must visit every child even after a hit: later callbacks may
    // replace contact data and update the query's nearest intersection.
    template<class Query>
    bool Query_Level(int level, Query&& query) const {
        bool result = false;
        Visit_Level(level, [&](const Attachment& attachment, int) {
            result |= query(attachment);
        });
        return result;
    }

    std::size_t Count() const noexcept {
        std::size_t count = m_additional.size();
        for (const auto& level : m_levels) count += level.size();
        return count;
    }
    const Attachment* At(std::size_t index) const noexcept {
        for (const auto& level : m_levels) {
            if (index < level.size()) return &level[index];
            index -= level.size();
        }
        return index < m_additional.size() ? &m_additional[index] : nullptr;
    }
    std::size_t Count_On_Bone(int bone) const {
        std::size_t count = 0;
        Visit_All([&](const Attachment& attachment, int) {
            if (attachment.bone == bone) ++count;
        });
        return count;
    }
    const Attachment* On_Bone(std::size_t index, int bone) const {
        return Find([&](const Attachment& attachment) {
            if (attachment.bone != bone) return false;
            if (index == 0) return true;
            --index;
            return false;
        });
    }
    template<class Predicate>
    const Attachment* Find(Predicate&& predicate) const {
        for (const auto& level : m_levels)
            for (const auto& attachment : level)
                if (predicate(attachment)) return &attachment;
        for (const auto& attachment : m_additional)
            if (predicate(attachment)) return &attachment;
        return nullptr;
    }

    // Extraction removes the first matching slot before scene callbacks run.
    // The returned owner keeps the resource alive until callbacks finish.
    template<class Predicate>
    std::optional<RemovedAttachment> Extract_First(Predicate&& predicate) {
        for (int level = 0; level < Level_Count(); ++level) {
            auto result = Extract(m_levels[level], level, predicate);
            if (result) return result;
        }
        return Extract(m_additional, -1, predicate);
    }

    template<class Removed, class Added>
    void Select_Level(int level, Removed&& removed, Added&& added) {
        if (m_levels.empty()) return;
        level = std::clamp(level, 0, Level_Count() - 1);
        if (level == m_current) return;
        for (const auto& attachment : m_levels[m_current]) removed(attachment);
        m_current = level;
        for (const auto& attachment : m_levels[m_current]) added(attachment);
    }

    // Clear in authored order, clearing each slot before detaching it. Release
    // that child's owner before proceeding to the next child's callback.
    template<class Detach>
    void Clear(Detach&& detach) {
        for (auto& level : m_levels) Clear_Level(level, detach);
        m_levels.clear();
        Clear_Level(m_additional, detach);
        m_current = 0;
    }

private:
    template<class Predicate>
    static std::optional<RemovedAttachment> Extract(std::vector<Attachment>& entries,
        int level, Predicate& predicate) {
        for (auto entry = entries.begin(); entry != entries.end(); ++entry) {
            if (!predicate(*entry)) continue;
            RemovedAttachment removed{std::move(*entry), level};
            entries.erase(entry);
            return removed;
        }
        return std::nullopt;
    }
    template<class Detach>
    static void Clear_Level(std::vector<Attachment>& entries, Detach& detach) {
        for (auto& entry : entries) {
            Owner model = std::move(entry.model);
            entry.model = Owner{};
            detach(model);
        }
        entries.clear();
    }
    std::vector<std::vector<Attachment>> m_levels;
    std::vector<Attachment> m_additional;
    int m_current = 0;
};
}
