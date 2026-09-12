module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

export module games.generalszh.gameplay.crates.definitions.collector_definition;
export import games.generalszh.gameplay.crates.definitions.crate_definition;

export namespace generalszh::crates
{
struct KindOfBinding final
{
	std::string name{};
	KindOfMask mask{};
};

class KindOfCatalog final
{
public:
	explicit KindOfCatalog(std::span<const KindOfBinding> bindings) : bindings_(bindings.begin(), bindings.end())
	{
		if (bindings_.empty())
			throw std::invalid_argument("KindOf catalog cannot be empty");
		std::sort(bindings_.begin(), bindings_.end(), [](const auto &left, const auto &right) {
			const auto leftUpper = CanonicalName(left.name);
			const auto rightUpper = CanonicalName(right.name);
			return leftUpper < rightUpper || (leftUpper == rightUpper && left.name < right.name);
		});
		for (std::size_t index = 0; index != bindings_.size(); ++index)
		{
			if (bindings_[index].name.empty() || bindings_[index].mask == 0)
				throw std::invalid_argument("KindOf binding requires a name and nonzero typed mask");
			if (index != 0 && EqualName(bindings_[index - 1].name, bindings_[index].name))
				throw std::invalid_argument("KindOf binding names must be unique");
		}
	}

	const KindOfBinding *Find(const std::string_view name) const noexcept
	{
		const auto it = std::find_if(bindings_.begin(), bindings_.end(),
			[&](const KindOfBinding &binding) { return EqualName(binding.name, name); });
		return it == bindings_.end() ? nullptr : &*it;
	}

private:
	static std::string CanonicalName(const std::string_view value)
	{
		std::string result;
		result.reserve(value.size());
		for (const char character : value)
			result.push_back(character >= 'a' && character <= 'z'
				? static_cast<char>(character - 'a' + 'A') : character);
		return result;
	}

	static bool EqualName(const std::string_view left, const std::string_view right) noexcept
	{
		if (left.size() != right.size())
			return false;
		for (std::size_t index = 0; index != left.size(); ++index)
		{
			const auto upper = [](const char character) noexcept {
				return character >= 'a' && character <= 'z' ? static_cast<char>(character - 'a' + 'A') : character;
			};
			if (upper(left[index]) != upper(right[index]))
				return false;
		}
		return true;
	}

	std::vector<KindOfBinding> bindings_;
};

// Runtime bindings are resolved by content. KindOf values never become
// mutable collector classification components.
struct CollectorDefinition final
{
	std::uint32_t key{};
	KindOfMask kindOf{};
	// This is an authored startup classification, not runtime height.  A true
	// value means the collector is explicitly bounded to the ground-only
	// veterancy path; unknown locomotor/height state remains ineligible.
	bool supportsGroundVeterancy{};
};

class CollectorDefinitionCatalog final
{
public:
	CollectorDefinitionCatalog(std::span<const CollectorDefinition> units,
		std::span<const CollectorDefinition> structures) :
		units_(units.begin(), units.end()), structures_(structures.begin(), structures.end())
	{
		Validate(units_);
		Validate(structures_);
	}

	const CollectorDefinition *Find(const bool structure, const std::uint32_t key) const noexcept
	{
		const auto &entries = structure ? structures_ : units_;
		const auto it = std::lower_bound(entries.begin(), entries.end(), key,
			[](const CollectorDefinition &entry, const std::uint32_t value) {
				return entry.key < value;
			});
		return it != entries.end() && it->key == key ? &*it : nullptr;
	}

private:
	static void Validate(const std::vector<CollectorDefinition> &entries)
	{
		if (!std::is_sorted(entries.begin(), entries.end(),
			[](const auto &left, const auto &right) { return left.key < right.key; }))
			throw std::invalid_argument("Collector definition keys must be sorted");
		if (std::adjacent_find(entries.begin(), entries.end(),
			[](const auto &left, const auto &right) { return left.key == right.key; }) != entries.end())
			throw std::invalid_argument("Collector definition keys must be unique");
	}

	std::vector<CollectorDefinition> units_;
	std::vector<CollectorDefinition> structures_;
};
} // namespace generalszh::crates
