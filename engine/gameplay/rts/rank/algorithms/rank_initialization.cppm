module;

#include <cstdint>
#include <limits>
#include <stdexcept>

export module engine.gameplay.rts.rank.algorithms.rank_initialization;
export import engine.gameplay.rts.rank.components.rank_state;
export import engine.gameplay.rts.rank.definitions.rank_definition;
export import engine.gameplay.rts.unlocks.components.unlock_state;

export namespace engine::gameplay::rts::rank
{
// Explicit account-enrollment setup. It borrows the already-created ECS state
// and immutable catalog; it does not own execution or schedule gameplay.
inline void InitializeRankAccount(RankState &rankState,
	 engine::gameplay::rts::unlocks::UnlockState &unlockState,
	 const RankCatalog &catalog, const std::uint64_t intrinsicCredits = 0)
{
	if (rankState.level != 0 || rankState.skillPoints != 0)
		throw std::logic_error("Rank account has already been initialized");

	const RankDefinition &initial = catalog.Get(1);
	if (intrinsicCredits > (std::numeric_limits<std::uint64_t>::max)() -
		initial.unlockCreditsGranted)
		throw std::overflow_error("Rank initialization credit overflow");
	const std::uint64_t addedCredits = intrinsicCredits + initial.unlockCreditsGranted;
	if (addedCredits > (std::numeric_limits<std::uint64_t>::max)() - unlockState.credits)
		throw std::overflow_error("Rank initialization credit overflow");

	for (const auto unlock : initial.unlocksGranted)
		if (!unlockState.IsAddressable(unlock))
			throw std::logic_error("Rank initialization references an unaddressable unlock ID");

	// Set only the rank-owned bits. Existing intrinsic/purchased ownership is
	// retained, matching legacy science de-duplication without clearing state.
	unlockState.credits += addedCredits;
	for (const auto unlock : initial.unlocksGranted)
		unlockState.SetOwned(unlock);
	rankState = RankState{0, 1};
}
} // namespace engine::gameplay::rts::rank
