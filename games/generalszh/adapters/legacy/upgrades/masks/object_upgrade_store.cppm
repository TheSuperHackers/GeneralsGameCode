module;

#include "object_upgrade_store.h"
#include "games/generalszh/adapters/legacy/upgrades/identity/upgrade_catalog.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

export module games.generalszh.adapters.legacy.upgrades.masks.object_upgrade_store;

import engine.ecs.core.world;
import engine.gameplay.rts.upgrades.state.upgrade_words;
import games.generalszh.simulation.gameplay_state;

extern "C++"
{
namespace generalszh::legacy
{
using engine::gameplay::rts::upgrades::CompletedUpgradeWord;
using engine::gameplay::rts::upgrades::UpgradeWordOrdinal;
using engine::gameplay::rts::upgrades::UpgradeWordOwner;

static constexpr std::size_t MaxRows = LegacyUpgradeWords{}.size();
static_assert(MaxRows == 8);
static_assert(UpgradeCatalogBridge::LegacyBitCount == MaxRows * 64);

struct ObjectUpgradeStore::Impl
{
	static constexpr auto None = (std::numeric_limits<std::size_t>::max)();

	struct Slot
	{
		std::array<ecs::Entity, MaxRows> rows{};
		std::size_t rowCount{};
		const UpgradeCatalogBridge *catalog{};
		std::uint64_t catalogHash{};
		std::size_t next{None};
		bool active{false};
	};

	struct MappedRows
	{
		std::array<CompletedUpgradeWord *, MaxRows> completed{};
		std::size_t rowCount{};
		std::size_t wordCount{};
		const UpgradeCatalogBridge *catalog{};
	};

	struct CatalogBinding
	{
		const UpgradeCatalogBridge *catalog{};
		std::uint64_t hash{};
		std::size_t wordCount{};
		std::size_t rowCount{};
	};

	GameplayState &state;
	std::vector<Slot> slots;
	std::size_t free{None};
	std::size_t count{};

	explicit Impl(GameplayState &injected) : state(injected) {}

	ecs::World &World() const noexcept
	{
		return state.World();
	}

	void EnsureWorldAccess() const
	{
		if (World().IsScheduledExecutionActive())
			throw std::logic_error(
				"Legacy object upgrade access cannot bypass scheduled component access");
	}

	CatalogBinding PrepareCatalog(const UpgradeCatalogBridge &catalog) const
	{
		// This is intentionally before selecting or appending a slot. A failed
		// catalog preflight therefore has no bookkeeping or ECS side effect.
		EnsureWorldAccess();
		if (!catalog.IsPublished())
			throw std::logic_error(
				"Object upgrades require an explicitly published catalog");
		const std::size_t wordCount = catalog.MaskWordCount();
		// Publish already validates the external native capacity. This is only
		// an internal invariant, not a second Release validation branch.
		assert(wordCount <= MaxRows);
		return CatalogBinding{
			&catalog,
			catalog.SchemaHash(),
			wordCount,
			wordCount == 0 ? 1 : wordCount};
	}

	MappedRows RequireRows(const std::size_t index) const
	{
		EnsureWorldAccess();
		if (index >= slots.size() || !slots[index].active)
			throw std::logic_error("Invalid object upgrade binding");

		const Slot &slot = slots[index];
		if (slot.catalog == nullptr || !slot.catalog->IsPublished())
			throw std::logic_error("Object upgrade catalog was released");
		if (slot.catalog->SchemaHash() != slot.catalogHash)
			throw std::logic_error("Object upgrade catalog schema changed");
		const std::size_t wordCount = slot.catalog->MaskWordCount();
		const std::size_t expectedRows = wordCount == 0 ? 1 : wordCount;
		assert(wordCount <= MaxRows);
		if (expectedRows != slot.rowCount)
			throw std::logic_error("Object upgrade catalog row count does not match binding");

		const ecs::Entity root = slot.rows[0];
		if (!root.IsValid() || !World().IsAlive(root))
			throw std::logic_error("Object upgrade owner binding is stale");

		MappedRows result{ {}, slot.rowCount, wordCount, slot.catalog };
		for (std::size_t ordinal = 0; ordinal < slot.rowCount; ++ordinal)
		{
			const ecs::Entity row = slot.rows[ordinal];
			if (!row.IsValid() || !World().IsAlive(row))
				throw std::logic_error("Object upgrade word binding is stale");

			auto *owner = World().Get<UpgradeWordOwner>(row);
			auto *wordOrdinal = World().Get<UpgradeWordOrdinal>(row);
			auto *completed = World().Get<CompletedUpgradeWord>(row);
			if (owner == nullptr || wordOrdinal == nullptr || completed == nullptr)
				throw std::logic_error("Object upgrade word is missing a required component");
			if (owner->value != root || wordOrdinal->value != ordinal)
				throw std::logic_error("Object upgrade word owner or ordinal does not match binding");
			result.completed[ordinal] = completed;
		}
		return result;
	}

	static void Import(const MappedRows &rows,
		const LegacyUpgradeWords &native,
		LegacyUpgradeWords &canonical)
	{
		canonical.fill(0);
		rows.catalog->ImportMask(
			std::span<const std::uint64_t>{native},
			std::span<std::uint64_t>{canonical}.first(rows.wordCount));
	}

	void CreateRows(Slot &slot)
	{
		assert(slot.rowCount >= 1 && slot.rowCount <= MaxRows);
		try
		{
			for (std::size_t ordinal = 0; ordinal < slot.rowCount; ++ordinal)
			{
				const ecs::Entity row =
					World().Create<UpgradeWordOwner, UpgradeWordOrdinal, CompletedUpgradeWord>();
				slot.rows[ordinal] = row;
				const ecs::Entity root = slot.rows[0];
				auto *owner = World().Get<UpgradeWordOwner>(row);
				auto *wordOrdinal = World().Get<UpgradeWordOrdinal>(row);
				assert(owner != nullptr && wordOrdinal != nullptr);
				owner->value = root;
				wordOrdinal->value = static_cast<std::uint32_t>(ordinal);
			}
		}
		catch (...)
		{
			DestroyRows(slot, true);
			throw;
		}
	}

	void DestroyRows(Slot &slot, const bool partial) noexcept
	{
		// The first row is the owner identity, so it must be destroyed last.
		for (std::size_t ordinal = slot.rowCount; ordinal != 0; --ordinal)
		{
			ecs::Entity &row = slot.rows[ordinal - 1];
			if (World().IsAlive(row))
			{
				if (!World().Destroy(row))
					std::terminate();
			}
			else if (!partial)
				std::terminate();
			row = {};
		}
		slot.rowCount = 0;
	}

	static void ResetSlot(Slot &slot, const std::size_t next) noexcept
	{
		slot.rows.fill({});
		slot.rowCount = 0;
		slot.catalog = nullptr;
		slot.catalogHash = 0;
		slot.next = next;
		slot.active = false;
	}
};

ObjectUpgradeStore::ObjectUpgradeStore(GameplayState &state) : m_impl(nullptr)
{
	if (state.World().IsScheduledExecutionActive())
		throw std::logic_error(
			"Legacy object upgrade construction cannot run during scheduled execution");
	m_impl = new Impl(state);
}

ObjectUpgradeStore::~ObjectUpgradeStore() noexcept
{
	if (m_impl->World().IsScheduledExecutionActive() || m_impl->count != 0)
		std::terminate();
	delete m_impl;
}

std::size_t ObjectUpgradeStore::Count() const noexcept
{
	return m_impl->count;
}

ObjectUpgradeLease::ObjectUpgradeLease(
	ObjectUpgradeStore &store,
	const UpgradeCatalogBridge &catalog) :
	m_store(store),
	m_slot(ObjectUpgradeStore::Impl::None)
{
	auto &impl = *store.m_impl;
	const auto binding = impl.PrepareCatalog(catalog);

	const bool append = impl.free == ObjectUpgradeStore::Impl::None;
	if (append)
		impl.slots.emplace_back();
	const std::size_t slotIndex = append ? impl.slots.size() - 1 : impl.free;
	auto &slot = impl.slots[slotIndex];
	const std::size_t previousNext = slot.next;
	ObjectUpgradeStore::Impl::ResetSlot(slot, previousNext);
	slot.rowCount = binding.rowCount;

	try
	{
		impl.CreateRows(slot);
	}
	catch (...)
	{
		// CreateRows has already removed every row it successfully created.
		ObjectUpgradeStore::Impl::ResetSlot(slot, previousNext);
		if (append)
			impl.slots.pop_back();
		throw;
	}

	if (!append)
		impl.free = previousNext;
	slot.catalog = binding.catalog;
	slot.catalogHash = binding.hash;
	slot.active = true;
	slot.next = ObjectUpgradeStore::Impl::None;
	m_slot = slotIndex;
	++impl.count;
}

ObjectUpgradeLease::~ObjectUpgradeLease() noexcept
{
	auto &impl = *m_store.m_impl;
	if (impl.World().IsScheduledExecutionActive() ||
		m_slot >= impl.slots.size() || !impl.slots[m_slot].active)
		std::terminate();

	auto &slot = impl.slots[m_slot];
	const std::size_t previousFree = impl.free;
	impl.DestroyRows(slot, false);
	ObjectUpgradeStore::Impl::ResetSlot(slot, previousFree);
	slot.next = previousFree;
	impl.free = m_slot;
	if (impl.count == 0)
		std::terminate();
	--impl.count;
}

LegacyUpgradeWords ObjectUpgradeLease::Read() const
{
	auto &impl = *m_store.m_impl;
	const auto rows = impl.RequireRows(m_slot);
	LegacyUpgradeWords canonical{};
	for (std::size_t ordinal = 0; ordinal < rows.rowCount; ++ordinal)
		canonical[ordinal] = rows.completed[ordinal]->value;
	for (std::size_t ordinal = rows.wordCount; ordinal < rows.rowCount; ++ordinal)
		if (canonical[ordinal] != 0)
			throw std::logic_error("Object upgrade state contains words outside the catalog");

	LegacyUpgradeWords native{};
	rows.catalog->ExportMask(
		std::span<const std::uint64_t>{canonical}.first(rows.wordCount),
		std::span<std::uint64_t>{native});
	return native;
}

void ObjectUpgradeLease::Add(const LegacyUpgradeWords &bits)
{
	auto &impl = *m_store.m_impl;
	const auto rows = impl.RequireRows(m_slot);
	LegacyUpgradeWords canonical{};
	ObjectUpgradeStore::Impl::Import(rows, bits, canonical);
	for (std::size_t ordinal = 0; ordinal < rows.wordCount; ++ordinal)
		rows.completed[ordinal]->value |= canonical[ordinal];
}

void ObjectUpgradeLease::Remove(const LegacyUpgradeWords &bits)
{
	auto &impl = *m_store.m_impl;
	const auto rows = impl.RequireRows(m_slot);
	LegacyUpgradeWords canonical{};
	ObjectUpgradeStore::Impl::Import(rows, bits, canonical);
	for (std::size_t ordinal = 0; ordinal < rows.wordCount; ++ordinal)
		rows.completed[ordinal]->value &= ~canonical[ordinal];
}

bool ObjectUpgradeLease::HasAll(const LegacyUpgradeWords &bits) const
{
	auto &impl = *m_store.m_impl;
	const auto rows = impl.RequireRows(m_slot);
	LegacyUpgradeWords canonical{};
	ObjectUpgradeStore::Impl::Import(rows, bits, canonical);
	for (std::size_t ordinal = 0; ordinal < rows.wordCount; ++ordinal)
		if ((canonical[ordinal] & ~rows.completed[ordinal]->value) != 0)
			return false;
	return true;
}

void ObjectUpgradeLease::Restore(const LegacyUpgradeWords &bits)
{
	auto &impl = *m_store.m_impl;
	const auto rows = impl.RequireRows(m_slot);
	LegacyUpgradeWords canonical{};
	// Import fully validates and fills the temporary canonical value before any
	// ECS column is touched, giving Restore its strong failure guarantee.
	ObjectUpgradeStore::Impl::Import(rows, bits, canonical);
	for (std::size_t ordinal = 0; ordinal < rows.wordCount; ++ordinal)
		rows.completed[ordinal]->value = canonical[ordinal];
}
}
}
