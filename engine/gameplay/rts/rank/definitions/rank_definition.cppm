module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

export module engine.gameplay.rts.rank.definitions.rank_definition;
export import engine.gameplay.rts.unlocks.definitions.unlock_catalog;

export namespace engine::gameplay::rts::rank
{
using RankSchemaHash = std::uint64_t;

inline constexpr std::string_view RankCatalogSchemaDomain =
	"engine.gameplay.rts.rank.catalog.schema";
inline constexpr std::uint32_t RankCatalogSchemaVersion = 1;

struct RankLevelLimit final
{
	std::uint32_t value{};
	friend constexpr bool operator==(const RankLevelLimit &, const RankLevelLimit &) noexcept = default;
};

struct RankDefinition final
{
	std::uint64_t skillPointsNeeded{};
	std::uint64_t unlockCreditsGranted{};
	std::vector<unlocks::UnlockId> unlocksGranted{};
};

// Immutable runtime rank definitions. Dense science IDs and the exact unlock
// catalog schema are captured at startup; runtime systems never resolve names.
class RankCatalog final
{
public:
	RankCatalog(std::span<const RankDefinition> definitions,
		const RankLevelLimit levelLimit, const unlocks::UnlockCatalog &unlockCatalog) :
		definitions_(definitions.begin(), definitions.end()), levelLimit_(levelLimit),
		unlockSchemaHash_(unlockCatalog.SchemaHash())
	{
		if (definitions_.empty() || definitions_.size() >
			static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()))
			throw std::invalid_argument("Rank catalog must contain a bounded definition set");
		if (levelLimit_.value == 0 || levelLimit_.value > definitions_.size())
			throw std::invalid_argument("Rank level limit must be within the rank catalog");
		if (definitions_.front().skillPointsNeeded != 0)
			throw std::invalid_argument("Rank skill thresholds must start at zero and be monotonic");
		for (std::size_t index = 1; index < definitions_.size(); ++index)
			if (definitions_[index - 1].skillPointsNeeded >= definitions_[index].skillPointsNeeded)
				throw std::invalid_argument("Rank skill thresholds must increase strictly");

		for (const RankDefinition &definition : definitions_)
		{
			for (std::size_t index = 0; index < definition.unlocksGranted.size(); ++index)
			{
				const unlocks::UnlockId id = definition.unlocksGranted[index];
				if (id == unlocks::InvalidUnlockId || id.value >= unlockCatalog.Count())
					throw std::invalid_argument("Rank definition references an invalid unlock ID");
				if (std::find(definition.unlocksGranted.begin(),
					definition.unlocksGranted.begin() + static_cast<std::ptrdiff_t>(index), id) !=
					definition.unlocksGranted.begin() + static_cast<std::ptrdiff_t>(index))
					throw std::invalid_argument("Rank definition grants the same unlock more than once");
			}
		}
		schemaHash_ = ComputeSchemaHash();
	}

	RankCatalog(const RankCatalog &) = delete;
	RankCatalog &operator=(const RankCatalog &) = delete;
	RankCatalog(RankCatalog &&) = delete;
	RankCatalog &operator=(RankCatalog &&) = delete;

	[[nodiscard]] std::size_t Count() const noexcept { return definitions_.size(); }
	[[nodiscard]] RankLevelLimit LevelLimit() const noexcept { return levelLimit_; }
	[[nodiscard]] unlocks::UnlockSchemaHash UnlockSchemaHash() const noexcept
	{
		return unlockSchemaHash_;
	}
	[[nodiscard]] RankSchemaHash SchemaHash() const noexcept { return schemaHash_; }

	[[nodiscard]] const RankDefinition &Get(const std::uint32_t level) const
	{
		if (level == 0 || level > definitions_.size())
			throw std::out_of_range("Rank level is outside the immutable catalog");
		return definitions_[static_cast<std::size_t>(level - 1)];
	}

private:
	static void HashByte(RankSchemaHash &hash, const std::uint8_t byte) noexcept
	{
		hash ^= static_cast<RankSchemaHash>(byte);
		hash *= UINT64_C(1099511628211);
	}

	static void HashBytes(RankSchemaHash &hash, const std::string_view bytes) noexcept
	{
		for (const char character : bytes)
			HashByte(hash, static_cast<std::uint8_t>(static_cast<unsigned char>(character)));
	}

	static void HashBigEndian(RankSchemaHash &hash, const std::uint64_t value,
		const std::size_t byteCount) noexcept
	{
		for (std::size_t index = byteCount; index > 0; --index)
			HashByte(hash, static_cast<std::uint8_t>(value >> ((index - 1) * 8)));
	}

	[[nodiscard]] RankSchemaHash ComputeSchemaHash() const noexcept
	{
		RankSchemaHash hash = UINT64_C(14695981039346656037);
		HashBytes(hash, RankCatalogSchemaDomain);
		HashBigEndian(hash, RankCatalogSchemaVersion, sizeof(std::uint32_t));
		HashBigEndian(hash, unlockSchemaHash_, sizeof(unlocks::UnlockSchemaHash));
		HashBigEndian(hash, levelLimit_.value, sizeof(std::uint32_t));
		HashBigEndian(hash, definitions_.size(), sizeof(std::uint64_t));
		for (const RankDefinition &definition : definitions_)
		{
			HashBigEndian(hash, definition.skillPointsNeeded, sizeof(std::uint64_t));
			HashBigEndian(hash, definition.unlockCreditsGranted, sizeof(std::uint64_t));
			HashBigEndian(hash, definition.unlocksGranted.size(), sizeof(std::uint64_t));
			for (const unlocks::UnlockId id : definition.unlocksGranted)
				HashBigEndian(hash, id.value, sizeof(std::uint32_t));
		}
		return hash;
	}

	std::vector<RankDefinition> definitions_;
	RankLevelLimit levelLimit_{};
	std::uint64_t unlockSchemaHash_{};
	RankSchemaHash schemaHash_{};
};
} // namespace engine::gameplay::rts::rank
