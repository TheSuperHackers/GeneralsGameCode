module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>

export module games.generalszh.gameplay.crates.algorithms.crate_veterancy_projection;
export import engine.ecs.core.entity;
export import engine.gameplay.progression.definitions.progression_definition;
export import engine.gameplay.progression.inputs.progression_batch;

export namespace generalszh::crates
{
struct ProjectedProgression final
{
	std::uint64_t experience{};
	std::uint32_t level{};
	bool valid{};
};

inline void RecomputeLevel(ProjectedProgression &state,
	const engine::gameplay::progression::ProgressionDefinition &definition) noexcept
{
	state.level = 0;
	while (state.level + 1 < definition.thresholds.size() &&
		state.experience >= definition.thresholds[state.level + 1])
		++state.level;
}

inline ProjectedProgression MakeProjection(const std::uint64_t experience,
	const std::uint32_t level,
	const bool trainable,
	const engine::gameplay::progression::ProgressionDefinition *definition) noexcept
{
	ProjectedProgression result{experience, level, trainable && definition != nullptr};
	if (result.valid && (experience > definition->maximumExperience ||
		level >= definition->thresholds.size()))
		result.valid = false;
	return result;
}

inline std::uint64_t ApplyExperience(ProjectedProgression &state,
	const engine::gameplay::progression::ProgressionDefinition &definition,
	const std::uint64_t requested) noexcept
{
	if (!state.valid)
		return 0;
	const auto room = definition.maximumExperience - state.experience;
	const auto applied = (std::min)(requested, room);
	state.experience += applied;
	RecomputeLevel(state, definition);
	return applied;
}

inline std::uint64_t ExperienceForLevels(const ProjectedProgression &state,
	const engine::gameplay::progression::ProgressionDefinition &definition,
	const std::uint32_t levels) noexcept
{
	if (!state.valid || levels == 0 || state.level >= definition.thresholds.size())
		return 0;
	const auto target = (std::min)(
		static_cast<std::uint64_t>(state.level) + levels,
		static_cast<std::uint64_t>(definition.thresholds.size() - 1));
	const auto targetExperience = definition.thresholds[target];
	return targetExperience > state.experience ? targetExperience - state.experience : 0;
}

inline std::uint64_t ApplyLevelGain(ProjectedProgression &state,
	const engine::gameplay::progression::ProgressionDefinition &definition,
	const std::uint32_t levels) noexcept
{
	const auto amount = ExperienceForLevels(state, definition, levels);
	if (!amount)
		return 0;
	return ApplyExperience(state, definition, amount);
}

inline void ApplyPrefix(ProjectedProgression &state,
	const engine::gameplay::progression::ProgressionDefinition &definition,
	std::span<const engine::gameplay::progression::AcceptedExperience> targetPrefix) noexcept
{
	// Callers pre-index PendingInputs() by entity in BeforeChunks.  This helper
	// deliberately accepts only the matching contiguous range, so projection
	// work is O(targets + awards), not O(targets * all-prefix-inputs).
	for (const auto &input : targetPrefix)
		(void)ApplyExperience(state, definition, input.amount);
}
} // namespace generalszh::crates
