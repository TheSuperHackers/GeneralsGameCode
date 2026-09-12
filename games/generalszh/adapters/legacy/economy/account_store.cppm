module;
#include "account_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <exception>
#include <limits>
#include <stdexcept>
#include <span>
#include <vector>
export module games.generalszh.adapters.legacy.economy.account_store;
import engine.ecs.core.world;
import engine.gameplay.rts.economy.components.resource_balance;
import games.generalszh.gameplay.economy.income_history;
import games.generalszh.gameplay.economy.transactions.account_transactions;
import games.generalszh.simulation.reset.persistent_world_reset;

extern "C++"
{
namespace generalszh::legacy
{
using engine::gameplay::rts::economy::ResourceBalance;
using economy::IncomeHistory;
namespace account_detail
{
IncomeHistory ToHistory(const AccountValue &value) { return {value.buckets, value.current, value.total}; }
void CopyHistory(AccountValue &value, const IncomeHistory &history)
{
	value.buckets = history.buckets;
	value.current = history.current;
	value.total = history.total;
}
}
using account_detail::ToHistory;
using account_detail::CopyHistory;
std::uint32_t AccountValue::QuoteWithdrawal(std::uint32_t requested) const
{
	return economy::QuoteWithdrawal(ResourceBalance{balance}, requested);
}
void AccountValue::ApplyWithdrawal(std::uint32_t approved)
{
	ResourceBalance value{balance};
	economy::ApplyWithdrawal(value, approved);
	balance = economy::BalanceValue(value);
}
void AccountValue::ApplyDeposit(std::uint32_t amount, bool trackIncome)
{
	ResourceBalance value{balance};
	auto history = ToHistory(*this); // Configuration/transfer path only.
	economy::ApplyDeposit(value, history, amount, trackIncome);
	balance = economy::BalanceValue(value);
	CopyHistory(*this, history);
}
void AccountValue::Reset(std::uint32_t starting)
{
	ResourceBalance value;
	IncomeHistory history;
	economy::ResetAccount(value, history, starting);
	balance = economy::BalanceValue(value);
	CopyHistory(*this, history);
}
void AccountValue::RecordIncome(std::uint32_t amount)
{
	auto history = ToHistory(*this);
	economy::RecordIncome(history, amount);
	CopyHistory(*this, history);
}
void AccountValue::AdvanceBucket(std::uint32_t next)
{
	auto history = ToHistory(*this);
	economy::AdvanceIncomeBucket(history, next);
	CopyHistory(*this, history);
}
void AccountValue::ClearIncome() noexcept { buckets.fill(0); current = total = 0; }
void AccountValue::Validate() const { economy::ValidateIncomeHistory(ToHistory(*this)); }
struct AccountStore::Impl
{
	static constexpr auto None = (std::numeric_limits<std::size_t>::max)();
	struct Slot { ecs::Entity entity{}; std::size_t next{None}; bool active{false}; };
	GameplayState &state;
	std::vector<Slot> slots;
	std::size_t free{None}, count{0};
	bool failed{false};
	bool prepared{false};
	std::vector<AccountValue> resetValues;
	std::vector<ecs::Entity> resetEntities;
	explicit Impl(GameplayState &injected) : state(injected) {}
	ecs::World &World() const noexcept { return state.World(); }
	template<class T = ResourceBalance> T &Require(std::size_t index) const
	{
		if (failed) throw std::logic_error("Failed account reset cannot be continued");
		if (prepared) throw std::logic_error("Account access during persistent reset is forbidden");
		if (index >= slots.size() || !slots[index].active) throw std::logic_error("Invalid account binding");
		auto *value = World().Get<T>(slots[index].entity);
		if (!value) throw std::logic_error("Missing bound account entity/component");
		return *value;
	}
	AccountValue Read(std::size_t index) const
	{
		const auto quantity = economy::BalanceValue(Require(index));
		const auto &history = Require<IncomeHistory>(index);
		return {quantity, history.buckets, history.current, history.total};
	}
	void Write(std::size_t index, const AccountValue &value)
	{
		value.Validate();
		auto &balance = Require(index);
		auto &history = Require<IncomeHistory>(index);
		balance.quantity = value.balance;
		history = ToHistory(value);
	}
	std::span<const ecs::Entity> Capture()
	{
		if (failed || prepared) throw std::logic_error("Cannot capture unavailable account storage");
		resetValues.resize(slots.size());
		resetEntities.clear();
		resetEntities.reserve(count);
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				resetValues[index] = Read(index);
				resetValues[index].Validate();
				resetEntities.push_back(slots[index].entity);
			}
		prepared = true;
		return resetEntities;
	}
	void Release()
	{
		if (!prepared || failed) throw std::logic_error("Account reset was not prepared");
		for (auto &slot : slots)
			if (slot.active)
			{
				if (!World().Destroy(slot.entity)) throw std::logic_error("Missing account during reset release");
				slot.entity = {};
			}
	}
	void Restore()
	{
		if (!prepared || failed) throw std::logic_error("Account reset was not prepared");
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				auto &slot = slots[index];
				slot.entity = World().Create<ResourceBalance, IncomeHistory>();
				World().Get<ResourceBalance>(slot.entity)->quantity = resetValues[index].balance;
				*World().Get<IncomeHistory>(slot.entity) = ToHistory(resetValues[index]);
			}
	}
	void Discard() noexcept { resetValues.clear(); resetEntities.clear(); prepared = false; }
};
AccountStore::AccountStore(GameplayState &state) : m_impl(new Impl(state)) {}
AccountStore::~AccountStore() noexcept
{
	if (Count() || m_impl->prepared) std::terminate();
	delete m_impl;
}
std::size_t AccountStore::Count() const noexcept { return m_impl->count; }
AccountLease::AccountLease(AccountStore &store, std::uint32_t initial) : AccountLease(store, AccountValue{initial}) {}
AccountLease::AccountLease(AccountStore &store, const AccountValue &initial) : m_store(store)
{
	initial.Validate();
	auto &impl = *store.m_impl;
	if (impl.failed || impl.prepared) throw std::logic_error("Cannot bind to unavailable account storage");
	const bool append = impl.free == AccountStore::Impl::None;
	if (append) impl.slots.emplace_back(); // All allocating bookkeeping precedes entity creation.
	m_slot = append ? impl.slots.size() - 1 : impl.free;
	ecs::Entity entity;
	try { entity = impl.World().Create<ResourceBalance, IncomeHistory>(); }
	catch (...) { if (append) impl.slots.pop_back(); throw; }
	if (!append) impl.free = impl.slots[m_slot].next;
	impl.slots[m_slot] = {entity, AccountStore::Impl::None, true};
	impl.World().Get<ResourceBalance>(entity)->quantity = initial.balance;
	*impl.World().Get<IncomeHistory>(entity) = ToHistory(initial);
	++impl.count;
}
AccountLease::~AccountLease() noexcept
{
	auto &impl = *m_store.m_impl;
	if (impl.prepared) std::terminate(); // Binding lifetimes cannot change during reset.
	auto &slot = impl.slots[m_slot];
	if (impl.World().IsAlive(slot.entity))
	{
		if (!impl.World().Destroy(slot.entity)) std::terminate();
	}
	else if (!impl.failed) std::terminate();
	slot = {{}, impl.free, false};
	impl.free = m_slot;
	--impl.count;
}
std::uint32_t AccountLease::Value() const
{
	return economy::BalanceValue(m_store.m_impl->Require(m_slot));
}
void AccountLease::Set(std::uint32_t value) { m_store.m_impl->Require(m_slot).quantity = value; }
std::uint32_t AccountLease::QuoteWithdrawal(std::uint32_t requested) const
{
	return economy::QuoteWithdrawal(m_store.m_impl->Require(m_slot), requested);
}
void AccountLease::ApplyWithdrawal(std::uint32_t approved)
{
	economy::ApplyWithdrawal(m_store.m_impl->Require(m_slot), approved);
}
void AccountLease::ApplyDeposit(std::uint32_t amount, bool trackIncome)
{
	auto &balance = m_store.m_impl->Require(m_slot);
	auto &history = m_store.m_impl->Require<IncomeHistory>(m_slot);
	economy::ApplyDeposit(balance, history, amount, trackIncome);
}
void AccountLease::Reset(std::uint32_t starting)
{
	auto &balance = m_store.m_impl->Require(m_slot);
	auto &history = m_store.m_impl->Require<IncomeHistory>(m_slot);
	economy::ResetAccount(balance, history, starting);
}
std::uint32_t AccountLease::IncomePerMinute() const { return m_store.m_impl->Require<IncomeHistory>(m_slot).total; }
void AccountLease::RecordIncome(std::uint32_t amount)
{
	economy::RecordIncome(m_store.m_impl->Require<IncomeHistory>(m_slot), amount);
}
void AccountLease::AdvanceBucket(std::uint32_t next)
{
	economy::AdvanceIncomeBucket(m_store.m_impl->Require<IncomeHistory>(m_slot), next);
}
void AccountLease::ClearIncome() { m_store.m_impl->Require<IncomeHistory>(m_slot) = {}; }
AccountValue AccountLease::State() const { return m_store.m_impl->Read(m_slot); }
void AccountLease::Restore(const AccountValue &value) { m_store.m_impl->Write(m_slot, value); }
void AccountStore::ResetWorld()
{
	const auto participant = ResetParticipant();
	PersistentWorldReset::Execute(m_impl->state, std::span{&participant, 1});
}
PersistentResetParticipant AccountStore::ResetParticipant()
{
	return {"games.generalszh.accounts", &m_impl->state, m_impl,
		+[](void *context) { return static_cast<Impl *>(context)->Capture(); },
		+[](void *context) { static_cast<Impl *>(context)->Release(); },
		+[](void *context) { static_cast<Impl *>(context)->Restore(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->Discard(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->failed = true; }};
}
}
}
