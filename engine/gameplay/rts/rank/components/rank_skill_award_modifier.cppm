module;

#include <cstdint>
#include <stdexcept>
#include <string_view>

export module engine.gameplay.rts.rank.components.rank_skill_award_modifier;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::rank
{
// Per-account multiplier for authored player skill awards.  The rational form
// keeps the authoritative calculation independent of platform floating-point
// behavior; account enrollment validates denominator before execution.
struct RankSkillAwardModifier final
{
	std::uint32_t numerator{1};
	std::uint32_t denominator{1};
};

inline bool IsValidRankSkillAwardModifier(const RankSkillAwardModifier &modifier) noexcept
{
	return modifier.denominator != 0;
}

inline void ValidateRankSkillAwardModifier(const RankSkillAwardModifier &modifier)
{
	if (!IsValidRankSkillAwardModifier(modifier))
		throw std::invalid_argument("Rank skill-award modifier denominator must be positive");
}
} // namespace engine::gameplay::rts::rank

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::rank::RankSkillAwardModifier>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.rank.skill_award_modifier";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
} // namespace ecs
