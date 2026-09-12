module;
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module games.generalszh.adapters.content.prerequisites.prerequisite_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import engine.gameplay.rts.production.definitions.prerequisite_definition;

export namespace generalszh::content
{
struct NamedPrerequisiteGroup
{
    std::vector<std::string> alternatives;
};

struct DecodedPrerequisites
{
    std::vector<NamedPrerequisiteGroup> objectGroups;
    // Science is retained as a named authoring reference until the
    // science-aware content boundary binds it to the root-owned unlock
    // catalog. The object-only binder below must never silently discard it.
    std::vector<std::string> scienceNames;

    bool Empty() const noexcept { return objectGroups.empty() && scienceNames.empty(); }
};

struct ContentDefinitionIdentity
{
    std::string_view name{};
    std::uint32_t key{};
};

inline constexpr std::size_t ZeroHourObjectPrerequisiteLimit = 32;

// The legacy parser makes one Object field an OR group and each separate
// prerequisite record an AND term. Science is decoded as a named reference,
// but object-only binding remains an explicit rejection until the consumer
// supplies a finalized science catalog.
inline DecodedPrerequisites DecodePrerequisites(const IniBlock &object)
{
    DecodedPrerequisites result;
    for (const auto &child : object.children)
    {
        if (!EqualToken(child.kind, "Prerequisites"))
            continue;
        for (const auto &field : child.fields)
        {
            if (EqualToken(field.key, "Science"))
            {
                const auto tokens = Tokens(field.value);
                if (tokens.size() != 1 || EqualToken(tokens.front(), "None"))
                    throw std::invalid_argument("Science prerequisite requires exactly one named science");
                result.scienceNames.emplace_back(tokens.front());
                continue;
            }
            if (!EqualToken(field.key, "Object"))
                throw std::invalid_argument("Unsupported prerequisite field: " + field.key);
            const auto tokens = Tokens(field.value);
            if (tokens.empty())
                throw std::invalid_argument("Object prerequisite requires at least one alternative");
            // ProductionPrerequisite::MAX_PREREQ bounds the m_prereqUnits
            // array used for this one parsed Object record. Separate Object
            // records are separate AND terms and each has its own bound.
            if (tokens.size() > ZeroHourObjectPrerequisiteLimit)
                throw std::length_error("Zero Hour Object prerequisites exceed 32 alternatives");
            NamedPrerequisiteGroup group;
            group.alternatives.reserve(tokens.size());
            for (const auto token : tokens)
                group.alternatives.emplace_back(token);
            result.objectGroups.push_back(std::move(group));
        }
    }
    return result;
}

inline engine::gameplay::rts::production::PrerequisiteDefinition BindObjectPrerequisites(
    const DecodedPrerequisites &source, std::span<const ContentDefinitionIdentity> identities)
{
    if (!source.scienceNames.empty())
        throw std::invalid_argument("Science prerequisites require the science-aware production binder");
    for (std::size_t i = 0; i != identities.size(); ++i)
    {
        if (identities[i].name.empty())
            throw std::invalid_argument("Content prerequisite identity name cannot be empty");
        for (std::size_t j = 0; j != i; ++j)
            if (identities[i].name == identities[j].name || identities[i].key == identities[j].key)
                throw std::invalid_argument("Duplicate or colliding content prerequisite identity");
    }

    std::vector<engine::gameplay::rts::production::PrerequisiteGroup> groups;
    groups.reserve(source.objectGroups.size());
    for (const auto &sourceGroup : source.objectGroups)
    {
        if (sourceGroup.alternatives.empty())
            throw std::invalid_argument("Object prerequisite group cannot be empty");
        engine::gameplay::rts::production::PrerequisiteGroup group;
        group.alternatives.reserve(sourceGroup.alternatives.size());
        for (const auto &name : sourceGroup.alternatives)
        {
            const auto found = std::find_if(identities.begin(), identities.end(),
                [&](const auto &identity) { return identity.name == name; });
            if (found == identities.end())
                throw std::invalid_argument("Object prerequisite names an unbound content definition: " + name);
            group.alternatives.push_back(found->key);
        }
        groups.push_back(std::move(group));
    }
    return engine::gameplay::rts::production::PrerequisiteDefinition(groups);
}

inline engine::gameplay::rts::production::PrerequisiteDefinition BindPrerequisites(
    const DecodedPrerequisites &source, std::span<const ContentDefinitionIdentity> identities)
{
    if (!source.scienceNames.empty())
        throw std::invalid_argument("Science prerequisites require the science-aware production binder");
    return BindObjectPrerequisites(source, identities);
}
}
