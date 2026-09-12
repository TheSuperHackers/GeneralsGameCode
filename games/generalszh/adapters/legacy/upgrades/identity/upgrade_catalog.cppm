module;
#include "upgrade_catalog.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

export module games.generalszh.adapters.legacy.upgrades.identity.upgrade_catalog;
import games.generalszh.gameplay.upgrades.identity.upgrade_catalog;

extern "C++"
{
namespace generalszh::legacy
{
struct UpgradeCatalogBridge::Impl
{
	struct Published
	{
		upgrades::UpgradeCatalog catalog;
		std::array<std::uint32_t, LegacyBitCount> legacyToDense;
		std::vector<std::uint32_t> denseToLegacy;
		Published() { legacyToDense.fill(InvalidId); }
	};
	std::unique_ptr<Published> published;
	const Published &Require() const
	{
		if (!published) throw std::logic_error("Upgrade catalog has not been explicitly published");
		return *published;
	}
};

UpgradeCatalogBridge::UpgradeCatalogBridge() : m_impl(new Impl) {}
UpgradeCatalogBridge::~UpgradeCatalogBridge() noexcept { delete m_impl; }
bool UpgradeCatalogBridge::IsPublished() const noexcept { return m_impl->published != nullptr; }
void UpgradeCatalogBridge::RequireMutable() const
{
	if (IsPublished()) throw std::logic_error("Upgrade definitions are frozen for this simulation");
}
void UpgradeCatalogBridge::Reset() noexcept { m_impl->published.reset(); }

void UpgradeCatalogBridge::Publish(const std::span<const UpgradeIdentityInput> definitions)
{
	RequireMutable();
	if (definitions.size() > LegacyBitCount)
		throw std::length_error("Upgrade catalog exceeds legacy bit capacity");
	auto next = std::make_unique<Impl::Published>();
	std::array<bool, LegacyBitCount> occupied{};
	for (const auto &definition : definitions)
	{
		if (definition.legacyBit >= LegacyBitCount || occupied[definition.legacyBit])
			throw std::invalid_argument("Upgrade definitions have invalid or duplicate legacy bits");
		occupied[definition.legacyBit] = true;
		(void)next->catalog.Register(definition.name, definition.version);
	}
	next->catalog.Finalize();
	next->denseToLegacy.resize(definitions.size());
	for (const auto &definition : definitions)
	{
		const auto id = next->catalog.Find(upgrades::MakeUpgradeKey(definition.name));
		assert(id.value < next->denseToLegacy.size());
		next->legacyToDense[definition.legacyBit] = id.value;
		next->denseToLegacy[id.value] = definition.legacyBit;
	}
	m_impl->published = std::move(next);
}

std::size_t UpgradeCatalogBridge::Count() const { return m_impl->Require().catalog.Count(); }
std::size_t UpgradeCatalogBridge::MaskWordCount() const { return (Count() + 63) / 64; }
std::uint64_t UpgradeCatalogBridge::SchemaHash() const { return m_impl->Require().catalog.SchemaHash(); }
std::uint64_t UpgradeCatalogBridge::Key(const std::uint32_t id) const
{
	return m_impl->Require().catalog.Get(upgrades::UpgradeId{id}).key.value;
}
std::string_view UpgradeCatalogBridge::Name(const std::uint32_t id) const
{
	return m_impl->Require().catalog.Name(upgrades::UpgradeId{id});
}
std::uint32_t UpgradeCatalogBridge::DenseId(const std::uint32_t legacyBit) const
{
	const auto &state = m_impl->Require();
	if (legacyBit >= LegacyBitCount) throw std::out_of_range("Legacy upgrade bit is out of range");
	return state.legacyToDense[legacyBit];
}
std::uint32_t UpgradeCatalogBridge::LegacyBit(const std::uint32_t id) const
{
	const auto &state = m_impl->Require();
	if (id >= state.denseToLegacy.size()) throw std::out_of_range("Dense upgrade ID is out of range");
	return state.denseToLegacy[id];
}

void UpgradeCatalogBridge::ImportMask(const std::span<const std::uint64_t> legacyWords,
	const std::span<std::uint64_t> modernWords) const
{
	const auto &state = m_impl->Require();
	if (legacyWords.size() != LegacyBitCount / 64 || modernWords.size() != MaskWordCount())
		throw std::invalid_argument("Upgrade mask word counts do not match the published catalog");
	std::array<std::uint64_t, LegacyBitCount / 64> converted{};
	for (std::size_t word = 0; word < legacyWords.size(); ++word)
	{
		auto bits = legacyWords[word];
		while (bits != 0)
		{
			const auto bit = word * 64 + static_cast<unsigned>(std::countr_zero(bits));
			const auto id = state.legacyToDense[bit];
			if (id == InvalidId) throw std::invalid_argument("Upgrade mask contains an unregistered legacy bit");
			converted[id / 64] |= UINT64_C(1) << (id % 64);
			bits &= bits - 1;
		}
	}
	std::copy_n(converted.begin(), modernWords.size(), modernWords.begin());
}

void UpgradeCatalogBridge::ExportMask(const std::span<const std::uint64_t> modernWords,
	const std::span<std::uint64_t> legacyWords) const
{
	const auto &state = m_impl->Require();
	if (legacyWords.size() != LegacyBitCount / 64 || modernWords.size() != MaskWordCount())
		throw std::invalid_argument("Upgrade mask word counts do not match the published catalog");
	std::array<std::uint64_t, LegacyBitCount / 64> converted{};
	for (std::size_t word = 0; word < modernWords.size(); ++word)
	{
		auto bits = modernWords[word];
		while (bits != 0)
		{
			const auto id = word * 64 + static_cast<unsigned>(std::countr_zero(bits));
			if (id >= state.denseToLegacy.size())
				throw std::invalid_argument("Upgrade mask contains an unregistered dense ID");
			const auto bit = state.denseToLegacy[id];
			converted[bit / 64] |= UINT64_C(1) << (bit % 64);
			bits &= bits - 1;
		}
	}
	std::copy(converted.begin(), converted.end(), legacyWords.begin());
}
}
}
