module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>

export module games.generalszh.gameplay.status.definitions.disabled_processing_definition;

export namespace generalszh::status
{

enum class DisabledReason : std::uint8_t
{
	Default = 0,
	Hacked = 1,
	Emp = 2,
	Held = 3,
	Paralyzed = 4,
	Unmanned = 5,
	Underpowered = 6,
	Freefall = 7,
	Awestruck = 8,
	Brainwashed = 9,
	Subdued = 10,
	ScriptDisabled = 11,
	ScriptUnderpowered = 12
};

inline constexpr std::size_t DisabledReasonCount = 13;
inline constexpr std::uint32_t DisabledReasonSchemaVersion = 1;
inline constexpr std::uint32_t DisabledProcessingDefinitionVersion = 1;
inline constexpr std::uint16_t AllDisabledReasons = 0x1fff;

class DisabledProcessingDefinition
{
public:
	explicit DisabledProcessingDefinition(std::uint16_t allowedMask);
	explicit DisabledProcessingDefinition(std::span<const DisabledReason> reasons);

	DisabledProcessingDefinition(const DisabledProcessingDefinition &) = default;
	DisabledProcessingDefinition &operator=(const DisabledProcessingDefinition &) = delete;
	DisabledProcessingDefinition &operator=(DisabledProcessingDefinition &&) = delete;

	[[nodiscard]] std::uint16_t AllowedMask() const noexcept
	{
		return m_allowedMask;
	}

	[[nodiscard]] std::uint64_t Fingerprint() const noexcept
	{
		return m_fingerprint;
	}

private:
	const std::uint16_t m_allowedMask;
	const std::uint64_t m_fingerprint;
};

} // namespace generalszh::status

namespace generalszh::status_detail
{

constexpr std::string_view FingerprintDomain =
	"games.generalszh.status.disabled_processing.definition.recipe";
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

std::uint16_t CheckedAllowedMask(const std::uint16_t allowedMask)
{
	constexpr std::uint16_t unknownBits =
		static_cast<std::uint16_t>(~generalszh::status::AllDisabledReasons);
	if ((allowedMask & unknownBits) != 0u)
		throw std::invalid_argument(
			"DisabledProcessingDefinition allowed mask contains unknown bits");
	return allowedMask;
}

std::uint16_t MaskFromReasons(
	const std::span<const generalszh::status::DisabledReason> reasons)
{
	std::uint16_t mask = 0;
	for (const auto reason : reasons)
	{
		const auto id = static_cast<std::uint8_t>(reason);
		if (id >= generalszh::status::DisabledReasonCount)
			throw std::invalid_argument(
				"DisabledProcessingDefinition contains an unknown reason");
		mask |= static_cast<std::uint16_t>(std::uint16_t{1} << id);
	}
	return mask;
}

constexpr std::uint64_t CalculateFingerprint(const std::uint16_t allowedMask) noexcept
{
	std::uint64_t hash = FnvOffsetBasis;
	HashLittleEndian(hash, static_cast<std::uint64_t>(FingerprintDomain.size()),
		sizeof(std::uint64_t));
	HashBytes(hash, FingerprintDomain);
	HashLittleEndian(hash, generalszh::status::DisabledReasonSchemaVersion,
		sizeof(std::uint32_t));
	HashLittleEndian(hash, generalszh::status::DisabledProcessingDefinitionVersion,
		sizeof(std::uint32_t));
	HashLittleEndian(hash, allowedMask, sizeof(std::uint16_t));
	return hash;
}

} // namespace generalszh::status_detail

namespace generalszh::status
{

DisabledProcessingDefinition::DisabledProcessingDefinition(
	const std::uint16_t allowedMask)
	: m_allowedMask(status_detail::CheckedAllowedMask(allowedMask)),
	  m_fingerprint(status_detail::CalculateFingerprint(m_allowedMask))
{
}

DisabledProcessingDefinition::DisabledProcessingDefinition(
	const std::span<const DisabledReason> reasons)
	: m_allowedMask(status_detail::MaskFromReasons(reasons)),
	  m_fingerprint(status_detail::CalculateFingerprint(m_allowedMask))
{
}

} // namespace generalszh::status
