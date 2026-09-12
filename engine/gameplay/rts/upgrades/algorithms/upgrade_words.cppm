module;

#include <cstdint>

export module engine.gameplay.rts.upgrades.algorithms.upgrade_words;
export import engine.gameplay.rts.upgrades.components.upgrade_words;

export namespace engine::gameplay::rts::upgrades
{
inline void StartWord(InProgressUpgradeWord &inProgress, const std::uint64_t bits) noexcept
{
	inProgress.value |= bits;
}

inline void CompleteWord(InProgressUpgradeWord &inProgress,
	CompletedUpgradeWord &completed,
	const std::uint64_t bits) noexcept
{
	inProgress.value &= ~bits;
	completed.value |= bits;
}

inline void RemoveWord(InProgressUpgradeWord &inProgress,
	CompletedUpgradeWord &completed,
	const std::uint64_t bits) noexcept
{
	inProgress.value &= ~bits;
	completed.value &= ~bits;
}
}
