module;

#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

export module engine.gameplay.rts.unlocks.definitions.unlock_definition;

export namespace engine::gameplay::rts::unlocks
{
using UnlockSchemaHash = std::uint64_t;

inline constexpr std::string_view UnlockKeyDomain =
	"engine.gameplay.rts.unlock:";
inline constexpr std::string_view UnlockCatalogSchemaDomain =
	"engine.gameplay.rts.unlock.catalog.schema";
inline constexpr std::uint32_t UnlockCatalogSchemaVersion = 1;

// The component stores two or more fixed words rather than a single machine
// word. The bound is deliberately explicit and adapters must reject catalogs
// larger than it during startup, retaining a bounded ECS component layout.
inline constexpr std::size_t MaxUnlockDefinitions = 128;
inline constexpr std::size_t UnlockWordCount =
	(MaxUnlockDefinitions + 63u) / 64u;
static_assert(MaxUnlockDefinitions > 64u);

struct UnlockKey final
{
	using ValueType = std::uint64_t;

	static constexpr ValueType InvalidValue = (std::numeric_limits<ValueType>::max)();
	ValueType value{};

	friend constexpr bool operator==(const UnlockKey &, const UnlockKey &) noexcept = default;
	friend constexpr auto operator<=>(const UnlockKey &, const UnlockKey &) noexcept = default;
};

struct UnlockId final
{
	using ValueType = std::uint32_t;

	static constexpr ValueType InvalidValue = (std::numeric_limits<ValueType>::max)();
	ValueType value{InvalidValue};

	static constexpr UnlockId Invalid() noexcept { return UnlockId{InvalidValue}; }
	friend constexpr bool operator==(const UnlockId &, const UnlockId &) noexcept = default;
	friend constexpr auto operator<=>(const UnlockId &, const UnlockId &) noexcept = default;
};

inline constexpr UnlockKey InvalidUnlockKey{UnlockKey::InvalidValue};
inline constexpr UnlockId InvalidUnlockId{UnlockId::InvalidValue};

constexpr bool IsValidUnlockName(const std::string_view name) noexcept
{
	if (name.empty())
		return false;
	for (const char character : name)
	{
		const auto byte = static_cast<unsigned char>(character);
		const bool isLetter =
			(byte >= static_cast<unsigned char>('A') && byte <= static_cast<unsigned char>('Z')) ||
			(byte >= static_cast<unsigned char>('a') && byte <= static_cast<unsigned char>('z'));
		const bool isDigit = byte >= static_cast<unsigned char>('0') &&
			byte <= static_cast<unsigned char>('9');
		if (!isLetter && !isDigit && byte != static_cast<unsigned char>('_') &&
			byte != static_cast<unsigned char>('.') && byte != static_cast<unsigned char>('-'))
			return false;
	}
	return true;
}

constexpr UnlockKey MakeUnlockKey(const std::string_view name) noexcept
{
	std::uint64_t hash = UINT64_C(14695981039346656037);
	for (const char character : UnlockKeyDomain)
	{
		hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(character));
		hash *= UINT64_C(1099511628211);
	}
	for (const char character : name)
	{
		hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(character));
		hash *= UINT64_C(1099511628211);
	}
	return UnlockKey{hash};
}

// Authored data is mutable only while a startup catalog is being assembled.
// Once registered and finalized, UnlockCatalog owns canonical address-free
// entries and dense prerequisite IDs; this value never becomes ECS state.
struct UnlockDefinition final
{
	UnlockKey key{};
	std::string name{};
	std::uint32_t version{1};
	std::uint64_t cost{};
	bool grantable{true};
	std::vector<UnlockKey> prerequisiteKeys{};

	UnlockDefinition() = default;
	UnlockDefinition(std::string_view canonicalName,
		std::uint32_t schemaVersion = 1,
		std::uint64_t unlockCost = 0,
		bool canGrant = true,
		std::span<const UnlockKey> prerequisites = {}) :
		key(MakeUnlockKey(canonicalName)),
		name(canonicalName),
		version(schemaVersion),
		cost(unlockCost),
		grantable(canGrant),
		prerequisiteKeys(prerequisites.begin(), prerequisites.end())
	{
	}

	UnlockDefinition(UnlockKey canonicalKey,
		std::string_view canonicalName,
		std::uint32_t schemaVersion = 1,
		std::uint64_t unlockCost = 0,
		bool canGrant = true,
		std::span<const UnlockKey> prerequisites = {}) :
		key(canonicalKey),
		name(canonicalName),
		version(schemaVersion),
		cost(unlockCost),
		grantable(canGrant),
		prerequisiteKeys(prerequisites.begin(), prerequisites.end())
	{
	}
};
} // namespace engine::gameplay::rts::unlocks
