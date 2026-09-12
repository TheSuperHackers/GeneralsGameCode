module;

#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>

export module engine.gameplay.rts.bounty.algorithms.bounty_amount;
export import engine.gameplay.rts.bounty.components.bounty_policy;

export namespace engine::gameplay::rts::bounty
{
inline BountyRate NormalizeBountyRate(const std::uint32_t numerator,
	const std::uint32_t denominator)
{
	if (denominator == 0 || numerator > denominator)
		throw std::invalid_argument("Bounty rate must be between zero and one");
	if (numerator == 0)
		return {};
	const auto divisor = std::gcd(numerator, denominator);
	return {numerator / divisor, denominator / divisor};
}

constexpr bool GreaterBountyRate(const BountyRate left, const BountyRate right) noexcept
{
	return static_cast<std::uint64_t>(left.numerator) * right.denominator
		> static_cast<std::uint64_t>(right.numerator) * left.denominator;
}

// Integer equivalent of ceil(cost * numerator / denominator). Runtime
// definitions and persisted policy are bounded to a nonnegative rate <= 1.
inline std::uint32_t CalculateBounty(const std::uint32_t cost, const BountyRate rate)
{
	assert(IsValidBountyRate(rate));
	// Definitions are validated at startup and persisted policy is produced by
	// that validated catalog. Keep only division defined if malformed internal
	// state nevertheless crosses this boundary in a release build.
	const auto denominator = rate.denominator == 0 ? 1u : rate.denominator;
	const auto numerator = rate.numerator;
	if (cost == 0 || numerator == 0)
		return 0;

	const auto whole = cost / denominator;
	const auto remainder = cost % denominator;
	const auto fractionalProduct = static_cast<std::uint64_t>(remainder) * numerator;
	std::uint64_t result = static_cast<std::uint64_t>(whole) * numerator
		+ fractionalProduct / denominator;
	if (fractionalProduct % denominator != 0)
		++result;
	assert(result <= static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()));
	return static_cast<std::uint32_t>(result);
}
}
