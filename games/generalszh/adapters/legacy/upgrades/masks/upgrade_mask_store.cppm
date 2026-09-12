module;
#include "upgrade_mask_store.h"
#include "games/generalszh/adapters/legacy/upgrades/identity/upgrade_catalog.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <algorithm>
#include <exception>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

export module games.generalszh.adapters.legacy.upgrades.masks.upgrade_mask_store;
import engine.ecs.core.world;
import engine.gameplay.rts.upgrades.state.upgrade_words;
import games.generalszh.simulation.gameplay_state;
import games.generalszh.simulation.reset.persistent_world_reset;

extern "C++"
{
namespace generalszh::legacy
{
using namespace engine::gameplay::rts::upgrades;
static_assert(UpgradeCatalogBridge::LegacyBitCount == 8 * 64);

struct UpgradeMaskStore::Impl
{
	static constexpr auto None = (std::numeric_limits<std::size_t>::max)();
	struct Slot
	{
		ecs::Entity owner{};
		std::array<ecs::Entity, 8> words{};
		const UpgradeCatalogBridge *catalog{};
		std::uint64_t schema{};
		std::size_t next{None};
		bool bound{false};
		bool active{false};
	};
	struct Row { CompletedUpgradeWord *completed; InProgressUpgradeWord *progress; };
	using Rows = std::array<Row, 8>;
	GameplayState &state;
	std::vector<Slot> slots;
	std::size_t free{None}, count{};
	bool prepared{false}, failed{false};
	std::vector<UpgradeMaskSnapshot> resetValues; // Canonical, only during reset.
	std::vector<ecs::Entity> resetEntities;
	explicit Impl(GameplayState &value) : state(value) {}
	ecs::World &World() const noexcept { return state.World(); }
	void EnsureAvailable() const
	{
		if (failed || prepared || World().IsScheduledExecutionActive())
			throw std::logic_error("Upgrade masks unavailable during scheduled execution or failed/active reset");
	}
	Rows Require(const std::size_t index) const
	{
		EnsureAvailable();
		if (index >= slots.size() || !slots[index].active || !World().IsAlive(slots[index].owner))
			throw std::logic_error("Invalid upgrade mask owner binding");
		Rows result{};
		for (std::size_t word = 0; word < result.size(); ++word)
		{
			const auto entity = slots[index].words[word];
			auto *owner = World().Get<UpgradeWordOwner>(entity);
			auto *ordinal = World().Get<UpgradeWordOrdinal>(entity);
			result[word] = {World().Get<CompletedUpgradeWord>(entity), World().Get<InProgressUpgradeWord>(entity)};
			if (!owner || !ordinal || owner->value != slots[index].owner || ordinal->value != word ||
				!result[word].completed || !result[word].progress)
				throw std::logic_error("Missing or mismatched upgrade mask word binding");
		}
		return result;
	}
	static UpgradeMaskSnapshot Snapshot(const Rows &rows) noexcept
	{
		UpgradeMaskSnapshot value;
		for (std::size_t word = 0; word < rows.size(); ++word)
		{
			value.completed[word] = rows[word].completed->value;
			value.inProgress[word] = rows[word].progress->value;
		}
		return value;
	}
	static void Assign(const Rows &rows, const UpgradeMaskSnapshot &value) noexcept
	{
		for (std::size_t word = 0; word < rows.size(); ++word)
		{
			rows[word].completed->value = value.completed[word];
			rows[word].progress->value = value.inProgress[word];
		}
	}
	static bool Empty(const Rows &rows) noexcept
	{
		std::uint64_t any{};
		for (auto row : rows) any |= row.completed->value | row.progress->value;
		return any == 0;
	}
	// Empty sets have meaning before definitions exist. Nonempty sets may never
	// be interpreted against a released or different catalog. This is an adapter
	// lifecycle gate, not a lazy catalog publication or a gameplay backend switch.
	bool BindCatalog(Slot &slot, const Rows &rows) const
	{
		if (!slot.catalog->IsPublished())
		{
			if (!Empty(rows)) throw std::logic_error("Nonempty upgrade masks require their published catalog");
			return false;
		}
		const auto schema = slot.catalog->SchemaHash();
		if (slot.bound && slot.schema != schema)
			throw std::logic_error("Upgrade mask catalog changed without explicit mask reset");
		slot.schema = schema;
		slot.bound = true;
		return true;
	}
	LegacyUpgradeWords Import(Slot &slot, const Rows &rows, const LegacyUpgradeWords &bits) const
	{
		LegacyUpgradeWords canonical{};
		if (!BindCatalog(slot, rows))
		{
			if (std::any_of(bits.begin(), bits.end(), [](auto word) { return word != 0; }))
				throw std::logic_error("Nonempty upgrade input requires explicit catalog publication");
			return canonical;
		}
		slot.catalog->ImportMask(bits, std::span{canonical}.first(slot.catalog->MaskWordCount()));
		return canonical;
	}
	void CreateRows(Slot &slot)
	{
		slot.owner = World().Create();
		try
		{
			for (std::size_t word = 0; word < slot.words.size(); ++word)
			{
				const auto entity = World().Create<UpgradeWordOwner, UpgradeWordOrdinal, CompletedUpgradeWord, InProgressUpgradeWord>();
				slot.words[word] = entity;
				World().Get<UpgradeWordOwner>(entity)->value = slot.owner;
				World().Get<UpgradeWordOrdinal>(entity)->value = static_cast<std::uint32_t>(word);
			}
		}
		catch (...)
		{
			DestroyRows(slot, true);
			throw;
		}
	}
	void DestroyRows(Slot &slot, const bool tolerateMissing)
	{
		for (auto &entity : slot.words)
		{
			if (World().IsAlive(entity)) World().Destroy(entity);
			else if (!tolerateMissing) throw std::logic_error("Missing upgrade word during release");
			entity = {};
		}
		if (World().IsAlive(slot.owner)) World().Destroy(slot.owner);
		else if (!tolerateMissing) throw std::logic_error("Missing upgrade owner during release");
		slot.owner = {};
	}
	std::span<const ecs::Entity> Capture()
	{
		EnsureAvailable();
		resetValues.resize(slots.size());
		resetEntities.clear();
		resetEntities.reserve(count * 9);
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				resetValues[index] = Snapshot(Require(index));
				resetEntities.push_back(slots[index].owner);
				resetEntities.insert(resetEntities.end(), slots[index].words.begin(), slots[index].words.end());
			}
		prepared = true;
		return resetEntities;
	}
	void Release()
	{
		if (!prepared || failed) throw std::logic_error("Upgrade mask reset not prepared");
		for (auto &slot : slots) if (slot.active) DestroyRows(slot, false);
	}
	void RestoreRows()
	{
		if (!prepared || failed) throw std::logic_error("Upgrade mask reset not prepared");
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				auto &slot = slots[index];
				CreateRows(slot);
				for (std::size_t word = 0; word < slot.words.size(); ++word)
				{
					World().Get<CompletedUpgradeWord>(slot.words[word])->value = resetValues[index].completed[word];
					World().Get<InProgressUpgradeWord>(slot.words[word])->value = resetValues[index].inProgress[word];
				}
			}
	}
	void Discard() noexcept { resetValues.clear(); resetEntities.clear(); prepared = false; }
};

UpgradeMaskStore::UpgradeMaskStore(GameplayState &state) : m_impl(new Impl(state)) {}
UpgradeMaskStore::~UpgradeMaskStore() noexcept
{
	if (Count() || m_impl->prepared) std::terminate();
	delete m_impl;
}
std::size_t UpgradeMaskStore::Count() const noexcept { return m_impl->count; }
PersistentResetParticipant UpgradeMaskStore::ResetParticipant()
{
	return {"games.generalszh.upgrade_masks", &m_impl->state, m_impl,
		+[](void *p) { return static_cast<Impl *>(p)->Capture(); },
		+[](void *p) { static_cast<Impl *>(p)->Release(); },
		+[](void *p) { static_cast<Impl *>(p)->RestoreRows(); },
		+[](void *p) noexcept { static_cast<Impl *>(p)->Discard(); },
		+[](void *p) noexcept { static_cast<Impl *>(p)->failed = true; }};
}
UpgradeMaskLease::UpgradeMaskLease(UpgradeMaskStore &store, const UpgradeCatalogBridge &catalog) : m_store(store)
{
	auto &impl = *store.m_impl;
	impl.EnsureAvailable();
	const bool append = impl.free == UpgradeMaskStore::Impl::None;
	if (append) impl.slots.emplace_back();
	m_slot = append ? impl.slots.size() - 1 : impl.free;
	auto &slot = impl.slots[m_slot];
	try { impl.CreateRows(slot); }
	catch (...) { if (append) impl.slots.pop_back(); throw; }
	if (!append) impl.free = slot.next;
	slot.catalog = &catalog;
	slot.bound = false;
	slot.active = true;
	slot.next = UpgradeMaskStore::Impl::None;
	++impl.count;
}
UpgradeMaskLease::~UpgradeMaskLease() noexcept
{
	auto &impl = *m_store.m_impl;
	if (impl.prepared || impl.World().IsScheduledExecutionActive()) std::terminate();
	auto &slot = impl.slots[m_slot];
	impl.DestroyRows(slot, impl.failed);
	slot = {};
	slot.next = impl.free;
	impl.free = m_slot;
	--impl.count;
}
UpgradeMaskSnapshot UpgradeMaskLease::Read() const
{
	auto &impl = *m_store.m_impl;
	auto rows = impl.Require(m_slot);
	auto &slot = impl.slots[m_slot];
	UpgradeMaskSnapshot native;
	if (!impl.BindCatalog(slot, rows)) return native;
	const auto canonical = impl.Snapshot(rows);
	const auto words = slot.catalog->MaskWordCount();
	// Reject corrupt unused columns too; the public bridge checks padding in
	// the last active word. Never silently truncate authoritative ECS bits.
	for (std::size_t word = words; word < rows.size(); ++word)
		if ((canonical.completed[word] | canonical.inProgress[word]) != 0)
			throw std::logic_error("Upgrade state contains words outside the catalog");
	slot.catalog->ExportMask(std::span{canonical.completed}.first(words), native.completed);
	slot.catalog->ExportMask(std::span{canonical.inProgress}.first(words), native.inProgress);
	return native;
}
void UpgradeMaskLease::Clear()
{
	auto &impl = *m_store.m_impl;
	impl.Assign(impl.Require(m_slot), {});
	impl.slots[m_slot].bound = false;
}
void UpgradeMaskLease::Start(const LegacyUpgradeWords &bits)
{
	auto &impl = *m_store.m_impl;
	auto rows = impl.Require(m_slot);
	const auto canonical = impl.Import(impl.slots[m_slot], rows, bits);
	for (std::size_t word = 0; word < rows.size(); ++word) StartWord(*rows[word].progress, canonical[word]);
}
void UpgradeMaskLease::Complete(const LegacyUpgradeWords &bits)
{
	auto &impl = *m_store.m_impl;
	auto rows = impl.Require(m_slot);
	const auto canonical = impl.Import(impl.slots[m_slot], rows, bits);
	for (std::size_t word = 0; word < rows.size(); ++word) CompleteWord(*rows[word].progress, *rows[word].completed, canonical[word]);
}
void UpgradeMaskLease::Remove(const LegacyUpgradeWords &bits)
{
	auto &impl = *m_store.m_impl;
	auto rows = impl.Require(m_slot);
	const auto canonical = impl.Import(impl.slots[m_slot], rows, bits);
	for (std::size_t word = 0; word < rows.size(); ++word) RemoveWord(*rows[word].progress, *rows[word].completed, canonical[word]);
}
void UpgradeMaskLease::MergeProgress(const UpgradeMaskLease &other)
{
	auto &impl = *m_store.m_impl;
	auto &source = *other.m_store.m_impl;
	const auto rows = impl.Require(m_slot);
	const auto sourceRows = source.Require(other.m_slot);
	auto &slot = impl.slots[m_slot];
	auto &sourceSlot = source.slots[other.m_slot];
	const bool destinationReady = impl.BindCatalog(slot, rows);
	const bool sourceReady = source.BindCatalog(sourceSlot, sourceRows);
	if (!destinationReady || !sourceReady)
	{
		for (auto row : sourceRows)
			if (row.progress->value != 0)
				throw std::logic_error("Upgrade transfer requires published catalogs");
		return;
	}
	if (slot.schema != sourceSlot.schema)
		throw std::invalid_argument("Upgrade transfer requires matching canonical schemas");
	// Canonical word indices match even if native bits differ. Addresses and
	// allocation order never determine identity or merge order.
	for (std::size_t word = 0; word < rows.size(); ++word)
		StartWord(*rows[word].progress, sourceRows[word].progress->value);
}
bool UpgradeMaskLease::HasCompleted(const LegacyUpgradeWords &bits) const
{
	auto &impl = *m_store.m_impl;
	auto rows = impl.Require(m_slot);
	const auto canonical = impl.Import(impl.slots[m_slot], rows, bits);
	std::uint64_t missing{};
	for (std::size_t word = 0; word < rows.size(); ++word) missing |= canonical[word] & ~rows[word].completed->value;
	return missing == 0;
}
bool UpgradeMaskLease::HasInProgress(const LegacyUpgradeWords &bits) const
{
	auto &impl = *m_store.m_impl;
	auto rows = impl.Require(m_slot);
	const auto canonical = impl.Import(impl.slots[m_slot], rows, bits);
	std::uint64_t missing{};
	for (std::size_t word = 0; word < rows.size(); ++word) missing |= canonical[word] & ~rows[word].progress->value;
	return missing == 0;
}
void UpgradeMaskLease::Restore(const UpgradeMaskSnapshot &snapshot)
{
	auto &impl = *m_store.m_impl;
	auto rows = impl.Require(m_slot);
	auto &slot = impl.slots[m_slot];
	const UpgradeMaskSnapshot canonical{impl.Import(slot, rows, snapshot.completed), impl.Import(slot, rows, snapshot.inProgress)};
	impl.Assign(rows, canonical);
}
}
}
