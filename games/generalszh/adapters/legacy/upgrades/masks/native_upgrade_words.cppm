module;
#include "upgrade_mask_store.h"
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>

export module games.generalszh.adapters.legacy.upgrades.masks.native_upgrade_words;

// Native BitFlags interop only. This is neither canonical ID remapping nor a
// wire/save codec. The catalog performs identity conversion separately.
export namespace generalszh::legacy
{
template<class Mask>
LegacyUpgradeWords ToLegacyWords(const Mask &mask)
{
	assert(static_cast<std::size_t>(mask.size()) == LegacyUpgradeWords{}.size() * 64);
	LegacyUpgradeWords words{};
	for (std::size_t bit = 0; bit < words.size() * 64; ++bit)
		words[bit / 64] |= static_cast<std::uint64_t>(mask.test(static_cast<int>(bit))) << (bit % 64);
	return words;
}

template<class Mask>
Mask FromLegacyWords(const LegacyUpgradeWords &words)
{
	Mask mask;
	assert(static_cast<std::size_t>(mask.size()) == words.size() * 64);
	for (std::size_t word = 0; word < words.size(); ++word)
	{
		auto bits = words[word];
		while (bits != 0)
		{
			mask.set(static_cast<int>(word * 64 + std::countr_zero(bits)));
			bits &= bits - 1;
		}
	}
	return mask;
}
}
