module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

export module games.generalszh.gameplay.production.definitions.production_definition;

namespace generalszh::production_detail
{
// This domain identifies only the production recipe fingerprint. It is not a
// process-wide schema key, content hash, or multiplayer identity.
constexpr std::string_view RecipeFingerprintDomain =
	"games.generalszh.production.production_definition.recipe";
constexpr std::uint64_t FnvOffsetBasis = UINT64_C(14695981039346656037);
constexpr std::uint64_t FnvPrime = UINT64_C(1099511628211);

void HashByte(std::uint64_t &hash, const std::uint8_t byte) noexcept
{
	hash ^= byte;
	hash *= FnvPrime;
}

void HashLittleEndian(std::uint64_t &hash, const std::uint64_t value,
	const std::size_t byteCount) noexcept
{
	for (std::size_t byte = 0; byte < byteCount; ++byte)
		HashByte(hash, static_cast<std::uint8_t>(value >> (byte * 8)));
}

void HashBytes(std::uint64_t &hash, const std::string_view bytes) noexcept
{
	for (const char byte : bytes)
		HashByte(hash, static_cast<std::uint8_t>(static_cast<unsigned char>(byte)));
}

std::size_t CheckedOffsetCount(const std::size_t ruleCount)
{
	const std::vector<std::size_t> offsetStorage;
	const std::vector<std::int32_t> quantityStorage;
	if (ruleCount > (std::numeric_limits<std::uint64_t>::max)())
		throw std::length_error(
			"ProductionDefinition rule count exceeds fingerprint length capacity");
	if (ruleCount > quantityStorage.max_size())
		throw std::length_error(
			"ProductionDefinition rule count exceeds quantity storage capacity");
	if (ruleCount >= offsetStorage.max_size())
		throw std::length_error(
			"ProductionDefinition rule count exceeds name offset storage capacity");
	return ruleCount + 1;
}

} // namespace generalszh::production_detail

export namespace generalszh::production
{
struct QuantityRuleInput
{
	std::string_view templateName;
	std::int32_t quantity;
};

} // namespace generalszh::production

namespace generalszh::production_detail
{
std::size_t CheckedNameByteCount(
	const std::span<const generalszh::production::QuantityRuleInput> rules)
{
	const std::vector<char> nameStorage;
	std::size_t total = 0;
	for (const auto &rule : rules)
	{
		if (rule.templateName.empty())
			throw std::invalid_argument(
				"ProductionDefinition template names must not be empty");
		if (rule.templateName.find('\0') != std::string_view::npos)
			throw std::invalid_argument(
				"ProductionDefinition template names must not contain NUL bytes");
		if (rule.quantity < 0)
			throw std::invalid_argument(
				"ProductionDefinition quantities must be non-negative");
		if (rule.templateName.size() > nameStorage.max_size() - total)
			throw std::length_error(
				"ProductionDefinition name bytes exceed storage capacity");
		if (total > (std::numeric_limits<std::uint64_t>::max)() ||
			rule.templateName.size() >
			(std::numeric_limits<std::uint64_t>::max)() - total)
			throw std::length_error(
				"ProductionDefinition name bytes exceed fingerprint length capacity");
		total += rule.templateName.size();
	}
	return total;
}
} // namespace generalszh::production_detail

export namespace generalszh::production
{

class ProductionDefinition
{
public:
	static constexpr std::uint32_t Version = 1;

	explicit ProductionDefinition(std::uint32_t queueLimit,
		std::span<const QuantityRuleInput> rules);

	ProductionDefinition(const ProductionDefinition &) = default;
	// Copy construction preserves any existing views of the source. No
	// destructive move operation can empty a published immutable definition.
	ProductionDefinition &operator=(const ProductionDefinition &) = delete;
	ProductionDefinition &operator=(ProductionDefinition &&) noexcept = delete;

	std::uint32_t QueueLimit() const noexcept
	{
		return queueLimit_;
	}

	std::span<const std::int32_t> Quantities() const noexcept
	{
		return {quantities_.data(), quantities_.size()};
	}

	std::string_view Name(const std::size_t index) const noexcept
	{
		assert(index < quantities_.size() && index + 1 < nameOffsets_.size());
		const std::size_t begin = nameOffsets_[index];
		const std::size_t end = nameOffsets_[index + 1];
		return {nameBytes_.data() + begin, end - begin};
	}

	std::uint64_t Fingerprint() const noexcept
	{
		return fingerprint_;
	}

private:
	std::uint64_t CalculateFingerprint() const noexcept
	{
		std::uint64_t hash = production_detail::FnvOffsetBasis;
		production_detail::HashLittleEndian(hash,
			static_cast<std::uint64_t>(production_detail::RecipeFingerprintDomain.size()),
			sizeof(std::uint64_t));
		production_detail::HashBytes(hash,
			production_detail::RecipeFingerprintDomain);
		production_detail::HashLittleEndian(hash, Version, sizeof(Version));
		production_detail::HashLittleEndian(hash, queueLimit_, sizeof(queueLimit_));
		production_detail::HashLittleEndian(hash,
			static_cast<std::uint64_t>(quantities_.size()), sizeof(std::uint64_t));

		for (std::size_t index = 0; index < quantities_.size(); ++index)
		{
			const std::size_t begin = nameOffsets_[index];
			const std::size_t end = nameOffsets_[index + 1];
			production_detail::HashLittleEndian(hash,
				static_cast<std::uint64_t>(end - begin), sizeof(std::uint64_t));
			production_detail::HashBytes(hash,
				std::string_view{nameBytes_.data() + begin, end - begin});
			production_detail::HashLittleEndian(hash,
				static_cast<std::uint32_t>(quantities_[index]), sizeof(std::uint32_t));
		}
		return hash;
	}

private:
	std::uint32_t queueLimit_;
	std::vector<char> nameBytes_;
	std::vector<std::size_t> nameOffsets_;
	std::vector<std::int32_t> quantities_;
	std::uint64_t fingerprint_{0};
};
} // namespace generalszh::production

generalszh::production::ProductionDefinition::ProductionDefinition(
	const std::uint32_t queueLimit,
	const std::span<const generalszh::production::QuantityRuleInput> rules)
	: queueLimit_(queueLimit)
{
	const std::size_t offsetCount =
		generalszh::production_detail::CheckedOffsetCount(rules.size());
	const std::size_t nameByteCount =
		generalszh::production_detail::CheckedNameByteCount(rules);

	nameBytes_.reserve(nameByteCount);
	nameOffsets_.reserve(offsetCount);
	quantities_.reserve(rules.size());

	nameOffsets_.push_back(0);
	std::size_t nameOffset = 0;
	for (const auto &rule : rules)
	{
		nameBytes_.insert(nameBytes_.end(),
			rule.templateName.begin(), rule.templateName.end());
		nameOffset += rule.templateName.size();
		nameOffsets_.push_back(nameOffset);
		quantities_.push_back(rule.quantity);
	}
	fingerprint_ = CalculateFingerprint();
}
