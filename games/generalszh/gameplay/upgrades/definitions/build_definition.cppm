module;

#include <cassert>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>

export module games.generalszh.gameplay.upgrades.definitions.build_definition;

static_assert(std::numeric_limits<float>::is_iec559,
	"BuildDefinition fingerprinting requires IEC 559 float semantics");
static_assert(sizeof(float) == 4,
	"BuildDefinition fingerprinting requires four-byte float values");
static_assert(sizeof(float) == sizeof(std::uint32_t),
	"BuildDefinition fingerprinting requires a four-byte float bit encoding");

export namespace generalszh::upgrades
{
using Seconds = std::chrono::duration<float>;

class BuildDefinition final
{
public:
	static constexpr std::uint32_t Version = 1;

	explicit BuildDefinition(Seconds duration, std::int32_t cost);

	BuildDefinition(const BuildDefinition &) = default;
	BuildDefinition &operator=(const BuildDefinition &) = delete;
	BuildDefinition &operator=(BuildDefinition &&) = delete;

	Seconds Duration() const noexcept
	{
		return m_duration;
	}

	std::int32_t Cost() const noexcept
	{
		return m_cost;
	}

	std::uint64_t Fingerprint() const noexcept
	{
		return m_fingerprint;
	}

private:
	const Seconds m_duration;
	const std::int32_t m_cost;
	const std::uint64_t m_fingerprint;
};
} // namespace generalszh::upgrades

namespace generalszh::upgrades_detail
{
constexpr std::string_view FingerprintDomain =
	"games.generalszh.upgrades.build_definition.recipe";
constexpr std::uint64_t FnvOffsetBasis = UINT64_C(14695981039346656037);
constexpr std::uint64_t FnvPrime = UINT64_C(1099511628211);

constexpr void HashByte(std::uint64_t &hash, const std::uint8_t byte) noexcept
{
	hash ^= byte;
	hash *= FnvPrime;
}

constexpr void HashLittleEndian(std::uint64_t &hash, const std::uint64_t value,
	const std::size_t byteCount) noexcept
{
	for (std::size_t byte = 0; byte < byteCount; ++byte)
		HashByte(hash, static_cast<std::uint8_t>(value >> (byte * 8)));
}

constexpr void HashBytes(std::uint64_t &hash, const std::string_view bytes) noexcept
{
	for (const char byte : bytes)
		HashByte(hash, static_cast<std::uint8_t>(static_cast<unsigned char>(byte)));
}

constexpr std::uint32_t CanonicalFloatBits(const float value) noexcept
{
	const auto bits = std::bit_cast<std::uint32_t>(value);
	return (bits & 0x7fffffffu) == 0 ? std::uint32_t{0} : bits;
}

std::chrono::duration<float> ValidateDuration(
	const std::chrono::duration<float> duration)
{
	const float seconds = duration.count();
	const auto bits = std::bit_cast<std::uint32_t>(seconds);
	const auto magnitude = bits & 0x7fffffffu;
	if (magnitude >= 0x7f800000u || ((bits >> 31) != 0 && magnitude != 0))
		throw std::invalid_argument(
			"Upgrade build duration must be finite and non-negative");
	return generalszh::upgrades::Seconds{std::bit_cast<float>(magnitude == 0 ? 0u : bits)};
}

std::int32_t ValidateCost(const std::int32_t cost)
{
	if (cost < 0)
		throw std::invalid_argument("Upgrade build cost must be non-negative");
	return cost;
}

std::uint64_t CalculateFingerprint(
	const std::chrono::duration<float> duration, const std::int32_t cost) noexcept
{
	assert(std::isfinite(duration.count()) && duration.count() >= 0.0f && cost >= 0);

	std::uint64_t hash = FnvOffsetBasis;
	HashLittleEndian(hash, static_cast<std::uint64_t>(FingerprintDomain.size()),
		sizeof(std::uint64_t));
	HashBytes(hash, FingerprintDomain);
	HashLittleEndian(hash, generalszh::upgrades::BuildDefinition::Version,
		sizeof(generalszh::upgrades::BuildDefinition::Version));
	HashLittleEndian(hash, CanonicalFloatBits(duration.count()), sizeof(std::uint32_t));
	HashLittleEndian(hash, static_cast<std::uint32_t>(cost), sizeof(std::uint32_t));
	return hash;
}
} // namespace generalszh::upgrades_detail

generalszh::upgrades::BuildDefinition::BuildDefinition(
	const Seconds duration, const std::int32_t cost)
	: m_duration(generalszh::upgrades_detail::ValidateDuration(duration)),
	  m_cost(generalszh::upgrades_detail::ValidateCost(cost)),
	  m_fingerprint(generalszh::upgrades_detail::CalculateFingerprint(m_duration, m_cost))
{
}
