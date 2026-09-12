module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module engine.gameplay.rts.unlocks.definitions.unlock_catalog;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;

export namespace engine::gameplay::rts::unlocks
{
struct UnlockEntry final
{
	UnlockId id{};
	UnlockKey key{};
	std::uint32_t version{1};
	std::uint64_t cost{};
	std::uint32_t nameOffset{};
	std::uint32_t nameSize{};
	std::uint32_t prerequisiteOffset{};
	std::uint32_t prerequisiteCount{};
	bool grantable{true};
};

class UnlockCatalog final
{
public:
	using Entry = UnlockEntry;

	UnlockCatalog() = default;
	~UnlockCatalog() = default;
	UnlockCatalog(const UnlockCatalog &) = delete;
	UnlockCatalog &operator=(const UnlockCatalog &) = delete;
	UnlockCatalog(UnlockCatalog &&) = delete;
	UnlockCatalog &operator=(UnlockCatalog &&) = delete;

	void Register(UnlockDefinition definition);
	void Finalize();

	[[nodiscard]] bool IsFinalized() const noexcept { return finalized_; }
	[[nodiscard]] std::size_t Count() const noexcept
	{
		return finalized_ ? entries_.size() : pending_.size();
	}
	[[nodiscard]] UnlockSchemaHash SchemaHash() const;
	[[nodiscard]] UnlockId Find(UnlockKey key) const;
	[[nodiscard]] const Entry &Get(UnlockId id) const;
	[[nodiscard]] std::string_view Name(UnlockId id) const;
	[[nodiscard]] std::span<const UnlockId> Prerequisites(UnlockId id) const;
	[[nodiscard]] std::span<const Entry> Entries() const;

private:
	static UnlockSchemaHash ComputeSchemaHash(
		const std::vector<UnlockDefinition *> &ordered);
	void RequireFinalized() const;

	std::vector<UnlockDefinition> pending_;
	std::vector<Entry> entries_;
	std::vector<UnlockId> prerequisiteIds_;
	std::string nameBytes_;
	UnlockSchemaHash schemaHash_{};
	bool finalized_{false};
};
} // namespace engine::gameplay::rts::unlocks

namespace engine::gameplay::rts::unlocks_detail
{
constexpr std::uint64_t FnvOffsetBasis = UINT64_C(14695981039346656037);
constexpr std::uint64_t FnvPrime = UINT64_C(1099511628211);

void HashByte(std::uint64_t &hash, const std::uint8_t byte) noexcept
{
	hash ^= static_cast<std::uint64_t>(byte);
	hash *= FnvPrime;
}

void HashBytes(std::uint64_t &hash, const std::string_view bytes) noexcept
{
	for (const char character : bytes)
		HashByte(hash, static_cast<std::uint8_t>(static_cast<unsigned char>(character)));
}

void HashBigEndian(std::uint64_t &hash, const std::uint64_t value,
	const std::size_t byteCount) noexcept
{
	for (std::size_t index = byteCount; index > 0; --index)
		HashByte(hash, static_cast<std::uint8_t>(value >> ((index - 1) * 8)));
}

void ValidateDefinition(const engine::gameplay::rts::unlocks::UnlockDefinition &definition)
{
	using namespace engine::gameplay::rts::unlocks;
	if (!IsValidUnlockName(definition.name))
		throw std::invalid_argument("Unlock name must be a non-empty ASCII token containing only letters, digits, '.', '_' or '-'");
	if (definition.key == InvalidUnlockKey || definition.key != MakeUnlockKey(definition.name))
		throw std::invalid_argument("Unlock definition key does not match its canonical name");
	if (definition.version == 0)
		throw std::invalid_argument("Unlock definition version must be nonzero");
	for (const UnlockKey prerequisite : definition.prerequisiteKeys)
		if (prerequisite == InvalidUnlockKey)
			throw std::invalid_argument("Unlock prerequisite key cannot be invalid");
}
} // namespace engine::gameplay::rts::unlocks_detail

void engine::gameplay::rts::unlocks::UnlockCatalog::Register(UnlockDefinition definition)
{
	if (finalized_)
		throw std::logic_error("Cannot register an unlock after catalog finalization");
	using namespace unlocks_detail;
	ValidateDefinition(definition);
	for (const UnlockDefinition &existing : pending_)
	{
		if (existing.name == definition.name)
			throw std::logic_error("Duplicate unlock name: " + definition.name);
		if (existing.key == definition.key)
			throw std::logic_error("Unlock key collision between '" + existing.name + "' and '" + definition.name + "'");
	}
	pending_.push_back(std::move(definition));
}

void engine::gameplay::rts::unlocks::UnlockCatalog::Finalize()
{
	if (finalized_)
		return;
	if (pending_.size() > MaxUnlockDefinitions)
		throw std::length_error("Unlock catalog exceeds its fixed component capacity");

	std::vector<UnlockDefinition *> ordered;
	ordered.reserve(pending_.size());
	for (UnlockDefinition &definition : pending_)
	{
		unlocks_detail::ValidateDefinition(definition);
		ordered.push_back(&definition);
	}
	std::sort(ordered.begin(), ordered.end(), [](const UnlockDefinition *left, const UnlockDefinition *right) {
		if (left->key != right->key)
			return left->key < right->key;
		return left->name < right->name;
	});
	for (std::size_t index = 1; index < ordered.size(); ++index)
	{
		if (ordered[index - 1]->key == ordered[index]->key)
			throw std::logic_error("Duplicate or colliding unlock key");
	}

	std::vector<Entry> finalEntries;
	finalEntries.reserve(ordered.size());
	std::vector<UnlockId> finalPrerequisites;
	std::string finalNameBytes;
	std::size_t nameByteCount = 0;
	for (const UnlockDefinition *definition : ordered)
	{
		if (definition->name.size() >
			static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()) - nameByteCount)
			throw std::length_error("Unlock catalog names exceed addressable storage");
		nameByteCount += definition->name.size();
	}
	if (nameByteCount > static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()))
		throw std::length_error("Unlock catalog names exceed addressable storage");
	finalNameBytes.reserve(nameByteCount);

	for (std::size_t index = 0; index < ordered.size(); ++index)
	{
		const UnlockDefinition &definition = *ordered[index];
		std::vector<UnlockKey> prerequisiteKeys = definition.prerequisiteKeys;
		std::sort(prerequisiteKeys.begin(), prerequisiteKeys.end());
		prerequisiteKeys.erase(std::unique(prerequisiteKeys.begin(), prerequisiteKeys.end()), prerequisiteKeys.end());
		if (prerequisiteKeys.size() > static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()) ||
			finalPrerequisites.size() >
				static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()) - prerequisiteKeys.size())
			throw std::length_error("Unlock prerequisite storage exceeds addressable capacity");
		for (const UnlockKey prerequisite : prerequisiteKeys)
		{
			if (prerequisite == definition.key)
				throw std::invalid_argument("Unlock definition cannot depend on itself");
			const auto found = std::lower_bound(ordered.begin(), ordered.end(), prerequisite,
				[](const UnlockDefinition *candidate, const UnlockKey key) { return candidate->key < key; });
			if (found == ordered.end() || (*found)->key != prerequisite)
				throw std::invalid_argument("Unlock definition references an unknown prerequisite");
			finalPrerequisites.push_back(UnlockId{static_cast<std::uint32_t>(std::distance(ordered.begin(), found))});
		}

		finalEntries.push_back(Entry{
			UnlockId{static_cast<std::uint32_t>(index)},
			definition.key,
			definition.version,
			definition.cost,
			static_cast<std::uint32_t>(finalNameBytes.size()),
			static_cast<std::uint32_t>(definition.name.size()),
			0,
			0,
			definition.grantable});
		finalEntries.back().prerequisiteOffset =
			static_cast<std::uint32_t>(finalPrerequisites.size() - prerequisiteKeys.size());
		finalEntries.back().prerequisiteCount = static_cast<std::uint32_t>(prerequisiteKeys.size());
		finalNameBytes.append(definition.name);
	}

	std::vector<std::uint8_t> marks(finalEntries.size(), 0);
	std::function<void(std::size_t)> visit = [&](const std::size_t index) {
		if (marks[index] == 1)
			throw std::invalid_argument("Unlock prerequisite cycle detected");
		if (marks[index] == 2)
			return;
		marks[index] = 1;
		const Entry &entry = finalEntries[index];
		for (std::uint32_t offset = 0; offset < entry.prerequisiteCount; ++offset)
			visit(finalPrerequisites[entry.prerequisiteOffset + offset].value);
		marks[index] = 2;
	};
	for (std::size_t index = 0; index < finalEntries.size(); ++index)
		visit(index);

	const UnlockSchemaHash finalSchemaHash = ComputeSchemaHash(ordered);
	entries_.swap(finalEntries);
	prerequisiteIds_.swap(finalPrerequisites);
	nameBytes_.swap(finalNameBytes);
	schemaHash_ = finalSchemaHash;
	std::vector<UnlockDefinition>{}.swap(pending_);
	finalized_ = true;
}

engine::gameplay::rts::unlocks::UnlockSchemaHash
engine::gameplay::rts::unlocks::UnlockCatalog::ComputeSchemaHash(
	const std::vector<UnlockDefinition *> &ordered)
{
	std::uint64_t hash = unlocks_detail::FnvOffsetBasis;
	unlocks_detail::HashBytes(hash, UnlockCatalogSchemaDomain);
	unlocks_detail::HashBigEndian(hash, UnlockCatalogSchemaVersion, sizeof(std::uint32_t));
	unlocks_detail::HashBigEndian(hash, static_cast<std::uint64_t>(ordered.size()), sizeof(std::uint64_t));
	for (const UnlockDefinition *definition : ordered)
	{
		unlocks_detail::HashBigEndian(hash, definition->key.value, sizeof(std::uint64_t));
		unlocks_detail::HashBigEndian(hash, definition->version, sizeof(std::uint32_t));
		unlocks_detail::HashBigEndian(hash, definition->cost, sizeof(std::uint64_t));
		unlocks_detail::HashByte(hash, definition->grantable ? 1u : 0u);
		unlocks_detail::HashBigEndian(hash, static_cast<std::uint64_t>(definition->name.size()), sizeof(std::uint64_t));
		unlocks_detail::HashBytes(hash, definition->name);
		std::vector<UnlockKey> prerequisiteKeys = definition->prerequisiteKeys;
		std::sort(prerequisiteKeys.begin(), prerequisiteKeys.end());
		prerequisiteKeys.erase(std::unique(prerequisiteKeys.begin(), prerequisiteKeys.end()), prerequisiteKeys.end());
		unlocks_detail::HashBigEndian(hash, static_cast<std::uint64_t>(prerequisiteKeys.size()), sizeof(std::uint64_t));
		for (const UnlockKey prerequisite : prerequisiteKeys)
			unlocks_detail::HashBigEndian(hash, prerequisite.value, sizeof(std::uint64_t));
	}
	return hash;
}

void engine::gameplay::rts::unlocks::UnlockCatalog::RequireFinalized() const
{
	if (!finalized_)
		throw std::logic_error("Explicit unlock catalog finalization is required");
}

engine::gameplay::rts::unlocks::UnlockSchemaHash
engine::gameplay::rts::unlocks::UnlockCatalog::SchemaHash() const
{
	RequireFinalized();
	return schemaHash_;
}

engine::gameplay::rts::unlocks::UnlockId
engine::gameplay::rts::unlocks::UnlockCatalog::Find(const UnlockKey key) const
{
	RequireFinalized();
	const auto found = std::lower_bound(entries_.begin(), entries_.end(), key,
		[](const Entry &entry, const UnlockKey candidate) { return entry.key < candidate; });
	return found == entries_.end() || found->key != key ? InvalidUnlockId : found->id;
}

const engine::gameplay::rts::unlocks::UnlockEntry &
engine::gameplay::rts::unlocks::UnlockCatalog::Get(const UnlockId id) const
{
	RequireFinalized();
	if (static_cast<std::size_t>(id.value) >= entries_.size())
		throw std::out_of_range("Invalid dense unlock ID");
	return entries_[id.value];
}

std::string_view engine::gameplay::rts::unlocks::UnlockCatalog::Name(const UnlockId id) const
{
	const Entry &entry = Get(id);
	return std::string_view{nameBytes_.data() + entry.nameOffset, entry.nameSize};
}

std::span<const engine::gameplay::rts::unlocks::UnlockId>
engine::gameplay::rts::unlocks::UnlockCatalog::Prerequisites(const UnlockId id) const
{
	const Entry &entry = Get(id);
	if (entry.prerequisiteCount == 0)
		return {};
	return std::span<const UnlockId>{prerequisiteIds_.data() + entry.prerequisiteOffset,
		entry.prerequisiteCount};
}

std::span<const engine::gameplay::rts::unlocks::UnlockEntry>
engine::gameplay::rts::unlocks::UnlockCatalog::Entries() const
{
	RequireFinalized();
	return entries_;
}
