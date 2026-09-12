module;

#include <cassert>
#include <cstdint>
#include <limits>

export module engine.gameplay.rts.rank.algorithms.rank_skill_award;
export import engine.gameplay.rts.rank.components.rank_skill_award_modifier;

export namespace engine::gameplay::rts::rank
{
// Apply the legacy ceiling rule without floating point.  Products are split
// before multiplication: the denominator is uint32_t, so the residual product
// is bounded below uint64_t max.  A valid but excessively large result clamps
// to uint64_t max; malformed zero denominators are guarded at enrollment and
// use a defined denominator here so Release arithmetic cannot divide by zero.
inline std::uint64_t ApplyRankSkillAward(const std::uint64_t amount,
	const RankSkillAwardModifier &modifier) noexcept
{
	assert(IsValidRankSkillAwardModifier(modifier));
	if (amount == 0 || modifier.numerator == 0)
		return 0;

	const std::uint64_t denominator = modifier.denominator == 0 ? 1 : modifier.denominator;
	const std::uint64_t numerator = modifier.numerator;
	const auto maximum = (std::numeric_limits<std::uint64_t>::max)();
	std::uint64_t result = 0;

	const auto addProduct = [&](const std::uint64_t left, const std::uint64_t right) {
		if (left != 0 && right > maximum / left)
			return false;
		const auto product = left * right;
		if (result > maximum - product)
			return false;
		result += product;
		return true;
	};

	const std::uint64_t wholeMultiplier = numerator / denominator;
	const std::uint64_t fractionalMultiplier = numerator % denominator;
	if (!addProduct(amount, wholeMultiplier))
		return maximum;

	const std::uint64_t wholeAmount = amount / denominator;
	if (!addProduct(wholeAmount, fractionalMultiplier))
		return maximum;

	const std::uint64_t remainder = amount % denominator;
	const std::uint64_t residualProduct = remainder * fractionalMultiplier;
	const std::uint64_t residual = residualProduct == 0 ? 0
		: (residualProduct + denominator - 1) / denominator;
	if (result > maximum - residual)
		return maximum;
	return result + residual;
}
} // namespace engine::gameplay::rts::rank
