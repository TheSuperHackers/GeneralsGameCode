module;

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module games.generalszh.gameplay.upgrades.identity.upgrade_catalog;

export namespace generalszh::upgrades
{
using UpgradeSchemaHash = std::uint64_t;

inline constexpr std::string_view UpgradeKeyDomain =
	"games.generalszh.upgrade:";
inline constexpr std::string_view UpgradeCatalogSchemaDomain =
	"games.generalszh.upgrade.catalog.schema";
inline constexpr std::uint32_t UpgradeCatalogSchemaVersion = 1;

struct UpgradeKey final
{
	using ValueType = std::uint64_t;

	ValueType value{};

	friend constexpr bool operator==(const UpgradeKey &, const UpgradeKey &) noexcept = default;
	friend constexpr auto operator<=>(const UpgradeKey &, const UpgradeKey &) noexcept = default;
};

struct UpgradeId final
{
	using ValueType = std::uint32_t;
	static constexpr ValueType InvalidValue = (std::numeric_limits<ValueType>::max)();

	ValueType value{InvalidValue};

	static constexpr UpgradeId Invalid() noexcept { return UpgradeId{InvalidValue}; }

	friend constexpr bool operator==(const UpgradeId &, const UpgradeId &) noexcept = default;
	friend constexpr auto operator<=>(const UpgradeId &, const UpgradeId &) noexcept = default;
};

inline constexpr UpgradeId InvalidUpgradeId{UpgradeId::InvalidValue};

constexpr bool IsValidUpgradeName(std::string_view name) noexcept
{
	if (name.empty())
		return false;

	for (const char character : name)
	{
		const auto byte = static_cast<unsigned char>(character);
		const bool isLetter = (byte >= static_cast<unsigned char>('A') &&
			byte <= static_cast<unsigned char>('Z')) ||
			(byte >= static_cast<unsigned char>('a') &&
			byte <= static_cast<unsigned char>('z'));
		const bool isDigit = byte >= static_cast<unsigned char>('0') &&
			byte <= static_cast<unsigned char>('9');
		if (!isLetter && !isDigit && byte != static_cast<unsigned char>('_') &&
			byte != static_cast<unsigned char>('.') &&
			byte != static_cast<unsigned char>('-'))
			return false;
	}
	return true;
}

constexpr UpgradeKey MakeUpgradeKey(std::string_view name) noexcept
{
	std::uint64_t hash = UINT64_C(14695981039346656037);
	for (const char character : UpgradeKeyDomain)
	{
		hash ^= static_cast<std::uint64_t>(
			static_cast<unsigned char>(character));
		hash *= UINT64_C(1099511628211);
	}
	for (const char character : name)
	{
		hash ^= static_cast<std::uint64_t>(
			static_cast<unsigned char>(character));
		hash *= UINT64_C(1099511628211);
	}
	return UpgradeKey{hash};
}

// The descriptor retains the canonical key alongside the textual identity.
// Catalog registration creates this descriptor from name/version; the key
// constructor is also a checked boundary for adapters that already carry a
// canonical key.
struct UpgradeDescriptor final
{
	UpgradeKey key{};
	std::string_view name{};
	std::uint32_t version{1};

	explicit UpgradeDescriptor(std::string_view canonicalName,
		std::uint32_t schemaVersion = 1);
	explicit UpgradeDescriptor(UpgradeKey canonicalKey,
		std::string_view canonicalName, std::uint32_t schemaVersion = 1);
};

struct Entry final
{
	UpgradeId id{};
	UpgradeKey key{};
	std::uint32_t version{1};
	std::uint32_t nameOffset{};
	std::uint32_t nameSize{};
};

using UpgradeEntry = Entry;

class UpgradeCatalog final
{
public:
	using Entry = generalszh::upgrades::Entry;

	UpgradeCatalog() = default;
	~UpgradeCatalog() = default;

	UpgradeCatalog(const UpgradeCatalog &) = delete;
	UpgradeCatalog &operator=(const UpgradeCatalog &) = delete;
	UpgradeCatalog(UpgradeCatalog &&) = delete;
	UpgradeCatalog &operator=(UpgradeCatalog &&) = delete;

	void Register(std::string_view name, std::uint32_t version = 1);

	void Finalize();

	[[nodiscard]] bool IsFinalized() const noexcept { return m_finalized; }
	[[nodiscard]] std::size_t Count() const noexcept
	{
		return m_finalized ? m_entries.size() : m_pending.size();
	}

	[[nodiscard]] UpgradeSchemaHash SchemaHash() const;
	[[nodiscard]] UpgradeId Find(UpgradeKey key) const;
	[[nodiscard]] const Entry &Get(UpgradeId id) const;
	[[nodiscard]] std::string_view Name(UpgradeId id) const;
	[[nodiscard]] std::span<const Entry> Entries() const;

private:
	struct PendingEntry final
	{
		UpgradeKey key{};
		std::string name{};
		std::uint32_t version{1};
	};

	static UpgradeSchemaHash ComputeSchemaHash(
		const std::vector<PendingEntry *> &ordered) noexcept;
	void RequireFinalized() const;

	std::vector<PendingEntry> m_pending;
	std::vector<Entry> m_entries;
	std::string m_nameBytes;
	UpgradeSchemaHash m_schemaHash{};
	bool m_finalized{false};
};

using UpgradeRegistry = UpgradeCatalog;
} // namespace generalszh::upgrades

namespace generalszh::upgrades_detail
{
constexpr std::uint64_t FnvOffsetBasis = UINT64_C(14695981039346656037);
constexpr std::uint64_t FnvPrime = UINT64_C(1099511628211);

void ValidateUpgradeName(const std::string_view name)
{
	if (!generalszh::upgrades::IsValidUpgradeName(name))
		throw std::invalid_argument(
			"Upgrade name must be a non-empty ASCII token containing only "
			"letters, digits, '.', '_' or '-'");
}

void ValidateDescriptor(const generalszh::upgrades::UpgradeKey key,
	const std::string_view name)
{
	ValidateUpgradeName(name);
	if (key != generalszh::upgrades::MakeUpgradeKey(name))
		throw std::invalid_argument(
			"Upgrade descriptor key does not match its canonical name");
}

void HashByte(std::uint64_t &hash, const std::uint8_t byte) noexcept
{
	hash ^= static_cast<std::uint64_t>(byte);
	hash *= FnvPrime;
}

void HashBytes(std::uint64_t &hash, const std::string_view bytes) noexcept
{
	for (const char character : bytes)
		HashByte(hash, static_cast<std::uint8_t>(
			static_cast<unsigned char>(character)));
}

void HashBigEndian(std::uint64_t &hash, const std::uint64_t value,
	const std::size_t byteCount) noexcept
{
	for (std::size_t index = byteCount; index > 0; --index)
		HashByte(hash, static_cast<std::uint8_t>(value >> ((index - 1) * 8)));
}
} // namespace generalszh::upgrades_detail

generalszh::upgrades::UpgradeDescriptor::UpgradeDescriptor(
	const std::string_view canonicalName, const std::uint32_t schemaVersion)
	: key(generalszh::upgrades::MakeUpgradeKey(canonicalName)),
	  name(canonicalName),
	  version(schemaVersion)
{
	generalszh::upgrades_detail::ValidateUpgradeName(name);
}

generalszh::upgrades::UpgradeDescriptor::UpgradeDescriptor(
	const generalszh::upgrades::UpgradeKey canonicalKey,
	const std::string_view canonicalName,
	const std::uint32_t schemaVersion)
	: key(canonicalKey), name(canonicalName), version(schemaVersion)
{
	generalszh::upgrades_detail::ValidateDescriptor(key, name);
}

void generalszh::upgrades::UpgradeCatalog::Register(
	const std::string_view name, const std::uint32_t version)
{
	if (m_finalized)
		throw std::logic_error(
			"Cannot register an upgrade after catalog finalization");

	const UpgradeDescriptor descriptor{name, version};
	for (const PendingEntry &existing : m_pending)
	{
		if (existing.name == descriptor.name)
			throw std::logic_error(
				std::string("Duplicate upgrade name: ") +
				std::string(existing.name));
		if (existing.key == descriptor.key)
			throw std::logic_error(
				std::string("Upgrade key collision between '") +
				std::string(existing.name) +
				"' and '" + std::string(descriptor.name) + "'");
	}

	// Constructing the owning string before changing the vector provides the
	// collection strong guarantee if allocation fails.
	PendingEntry pending{descriptor.key, std::string(descriptor.name), descriptor.version};
	m_pending.push_back(std::move(pending));
}

void generalszh::upgrades::UpgradeCatalog::Finalize()
{
	if (m_finalized)
		return;

	// All final state is built in temporaries. No published view or lifecycle
	// flag changes until every validation, allocation, and hash operation has
	// completed successfully.
	std::vector<PendingEntry *> orderedByKey;
	orderedByKey.reserve(m_pending.size());
	for (PendingEntry &pending : m_pending)
	{
		generalszh::upgrades_detail::ValidateDescriptor(pending.key, pending.name);
		orderedByKey.push_back(&pending);
	}

	if (orderedByKey.size() > static_cast<std::size_t>(UpgradeId::InvalidValue))
		throw std::length_error("Too many upgrades for dense UpgradeId");

	std::sort(orderedByKey.begin(), orderedByKey.end(),
		[](const PendingEntry *left, const PendingEntry *right) {
			if (left->key != right->key)
				return left->key < right->key;
			return left->name < right->name;
		});
	for (std::size_t index = 1; index < orderedByKey.size(); ++index)
	{
		if (orderedByKey[index - 1]->key == orderedByKey[index]->key)
		{
			if (orderedByKey[index - 1]->name == orderedByKey[index]->name)
				throw std::logic_error(
					std::string("Duplicate upgrade name: ") +
					std::string(orderedByKey[index - 1]->name));
			throw std::logic_error(
				std::string("Upgrade key collision between '") +
				std::string(orderedByKey[index - 1]->name) + "' and '" +
				std::string(orderedByKey[index]->name) + "'");
		}
	}

	std::size_t nameByteCount = 0;
	const auto maximumStoredNameSize =
		static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)());
	for (const PendingEntry *pending : orderedByKey)
	{
		if (pending->name.size() > maximumStoredNameSize ||
			nameByteCount > maximumStoredNameSize - pending->name.size())
			throw std::length_error("Upgrade catalog name storage is too large");
		nameByteCount += pending->name.size();
	}

	std::vector<Entry> finalEntries;
	finalEntries.reserve(orderedByKey.size());
	std::string finalNameBytes;
	finalNameBytes.reserve(nameByteCount);
	for (std::size_t index = 0; index < orderedByKey.size(); ++index)
	{
		const PendingEntry &pending = *orderedByKey[index];
		finalEntries.push_back(Entry{
			UpgradeId{static_cast<std::uint32_t>(index)},
			pending.key,
			pending.version,
			static_cast<std::uint32_t>(finalNameBytes.size()),
			static_cast<std::uint32_t>(pending.name.size())});
		finalNameBytes.append(pending.name);
	}

	const UpgradeSchemaHash finalSchemaHash = ComputeSchemaHash(orderedByKey);
	m_entries.swap(finalEntries);
	m_nameBytes.swap(finalNameBytes);
	m_schemaHash = finalSchemaHash;
	std::vector<PendingEntry>{}.swap(m_pending);
	m_finalized = true;
}

generalszh::upgrades::UpgradeSchemaHash
generalszh::upgrades::UpgradeCatalog::ComputeSchemaHash(
	const std::vector<PendingEntry *> &ordered) noexcept
{
	std::uint64_t hash = generalszh::upgrades_detail::FnvOffsetBasis;
	generalszh::upgrades_detail::HashBytes(hash,
		generalszh::upgrades::UpgradeCatalogSchemaDomain);
	generalszh::upgrades_detail::HashBigEndian(hash,
		generalszh::upgrades::UpgradeCatalogSchemaVersion, sizeof(std::uint32_t));
	generalszh::upgrades_detail::HashBigEndian(hash,
		static_cast<std::uint64_t>(ordered.size()), sizeof(std::uint64_t));
	for (const PendingEntry *pending : ordered)
	{
		generalszh::upgrades_detail::HashBigEndian(hash, pending->key.value,
			sizeof(std::uint64_t));
		generalszh::upgrades_detail::HashBigEndian(hash, pending->version,
			sizeof(std::uint32_t));
		generalszh::upgrades_detail::HashBigEndian(hash,
			static_cast<std::uint64_t>(pending->name.size()), sizeof(std::uint64_t));
		generalszh::upgrades_detail::HashBytes(hash, pending->name);
	}
	return hash;
}

void generalszh::upgrades::UpgradeCatalog::RequireFinalized() const
{
	if (!m_finalized)
		throw std::logic_error(
			"Explicit upgrade catalog finalization is required");
}

generalszh::upgrades::UpgradeSchemaHash
generalszh::upgrades::UpgradeCatalog::SchemaHash() const
{
	RequireFinalized();
	return m_schemaHash;
}

generalszh::upgrades::UpgradeId generalszh::upgrades::UpgradeCatalog::Find(
	const generalszh::upgrades::UpgradeKey key) const
{
	RequireFinalized();
	const auto found = std::lower_bound(m_entries.begin(), m_entries.end(), key,
		[](const Entry &entry, const UpgradeKey value) {
			return entry.key < value;
		});
	if (found == m_entries.end() || found->key != key)
		return InvalidUpgradeId;
	return found->id;
}

const generalszh::upgrades::Entry &generalszh::upgrades::UpgradeCatalog::Get(
	const generalszh::upgrades::UpgradeId id) const
{
	RequireFinalized();
	if (static_cast<std::size_t>(id.value) >= m_entries.size())
		throw std::out_of_range("Invalid dense upgrade ID");
	return m_entries[id.value];
}

std::string_view generalszh::upgrades::UpgradeCatalog::Name(
	const generalszh::upgrades::UpgradeId id) const
{
	const Entry &entry = Get(id);
	return std::string_view{m_nameBytes.data() + entry.nameOffset, entry.nameSize};
}

std::span<const generalszh::upgrades::Entry>
generalszh::upgrades::UpgradeCatalog::Entries() const
{
	RequireFinalized();
	return std::span<const Entry>{m_entries};
}
