module;
#include "gameplay_state.h"
#include <exception>
#include <memory>
#include <stdexcept>
export module games.generalszh.simulation.gameplay_state;
import engine.ecs.core.world;
import engine.gameplay.lifetime.components.expiration;
import engine.gameplay.lifetime.components.expiration_eligibility;
import games.generalszh.gameplay.lifetime.components.lifetime_dispatch;
import engine.gameplay.rts.economy.components.periodic_income;
import engine.gameplay.rts.economy.components.income_dispatch;
import engine.gameplay.rts.economy.components.resource_balance;
import engine.gameplay.rts.power.ledger.power_ledger;
import engine.gameplay.rts.radar.availability.radar_availability;
import engine.gameplay.rts.radar.commands.radar_batch;
import engine.gameplay.rts.production.quantity.production_quantity;
import engine.gameplay.rts.production.queue.production_queue;
import games.generalszh.gameplay.power.suppression.power_suppression;
import games.generalszh.gameplay.power.recovery.power_recovery;
import games.generalszh.gameplay.economy.auto_deposit;
import games.generalszh.gameplay.economy.income.income_components;
import games.generalszh.gameplay.economy.capture.capture_batch;
import games.generalszh.gameplay.economy.income_history;
import games.generalszh.gameplay.production.progress.production_progress;
import games.generalszh.gameplay.production.entry.production_entry_metadata;
import games.generalszh.gameplay.production.entry.exit_reservation;
import games.generalszh.gameplay.production.doors.production_door;
import games.generalszh.gameplay.production.presentation.construction_marker;
import games.generalszh.presentation.model.pending_conditions;
import games.generalszh.gameplay.upgrades.state.upgrade_status;
import engine.gameplay.rts.upgrades.state.upgrade_words;
import engine.gameplay.rts.upgrades.state.upgrade_execution;

// Definitions match the global-module legacy ABI declarations in the header.
extern "C++"
{
namespace generalszh
{
struct GameplayState::Impl
{
	ecs::World world;
	Impl()
	{
		world.RegisterComponent<engine::gameplay::lifetime::Expiration>();
		world.RegisterComponent<engine::gameplay::lifetime::ExpirationEligibility>();
		world.RegisterComponent<lifetime::LifetimeTarget>();
		world.RegisterComponent<lifetime::LifetimeDispatch>();
		world.RegisterComponent<engine::gameplay::rts::economy::IncomeSchedule>();
		world.RegisterComponent<engine::gameplay::rts::economy::IncomeRate>();
		world.RegisterComponent<engine::gameplay::rts::economy::IncomeEligibility>();
		world.RegisterComponent<engine::gameplay::rts::economy::IncomeDispatch>();
		world.RegisterComponent<engine::gameplay::rts::economy::IncomePulse>();
		world.RegisterComponent<economy::CaptureRewardState>();
		world.RegisterComponent<economy::IncomeDefinition>();
		world.RegisterComponent<economy::IncomeSource>();
		world.RegisterComponent<economy::IncomeObservation>();
		world.RegisterComponent<economy::IncomePayout>();
		world.RegisterComponent<economy::CaptureRange>();
		world.RegisterComponent<economy::IncomeHistory>();
		world.RegisterComponent<engine::gameplay::rts::economy::ResourceBalance>();
		world.RegisterComponent<engine::gameplay::rts::power::PowerLedger>();
		world.RegisterComponent<power::PowerSuppression>();
		world.RegisterComponent<power::PowerRecoveryOwner>();
		world.RegisterComponent<power::PowerRecovery>();
		world.RegisterComponent<engine::gameplay::rts::radar::RadarAvailability>();
		world.RegisterComponent<engine::gameplay::rts::radar::RadarBatchRange>();
		world.RegisterComponent<engine::gameplay::rts::production::ProductionQuantity>();
		world.RegisterComponent<engine::gameplay::rts::production::ProductionQueue>();
		world.RegisterComponent<engine::gameplay::rts::production::ProductionQueueMember>();
		world.RegisterComponent<production::ProductionElapsed>();
		world.RegisterComponent<production::ProductionProgress>();
		world.RegisterComponent<production::ProductionEntryKind>();
		world.RegisterComponent<production::ProductionCorrelation>();
		world.RegisterComponent<production::ProductionExitReservation>();
		world.RegisterComponent<production::DoorOpening>();
		world.RegisterComponent<production::DoorWaiting>();
		world.RegisterComponent<production::DoorClosing>();
		world.RegisterComponent<production::DoorHold>();
		world.RegisterComponent<production::ConstructionMarker>();
		world.RegisterComponent<presentation::PendingConditionClear>();
		world.RegisterComponent<presentation::PendingConditionSet>();
		world.RegisterComponent<presentation::PendingConditionDirty>();
		world.RegisterComponent<upgrades::UpgradeInstanceStatus>();
		world.RegisterComponent<engine::gameplay::rts::upgrades::UpgradeWordOwner>();
		world.RegisterComponent<engine::gameplay::rts::upgrades::UpgradeWordOrdinal>();
		world.RegisterComponent<engine::gameplay::rts::upgrades::CompletedUpgradeWord>();
		world.RegisterComponent<engine::gameplay::rts::upgrades::InProgressUpgradeWord>();
		world.RegisterComponent<engine::gameplay::rts::upgrades::UpgradeExecutionState>();
		world.FinalizeComponents();
	}
};
GameplayState::GameplayState() : m_impl(new Impl) {}
GameplayState::~GameplayState() noexcept
{
	if (Count() != 0 || m_worldBindings != 0) std::terminate();
	delete m_impl;
}
ecs::World &GameplayState::World() noexcept { return m_impl->world; }
std::size_t GameplayState::Count() const noexcept { return m_impl->world.EntityCount(); }
void GameplayState::Reset()
{
	BeginReset();
	try { ReplaceEmptyWorld(); }
	catch (...) { EndReset(); throw; }
	EndReset();
}
void GameplayState::BeginReset()
{
	if (m_resetting.test_and_set(std::memory_order_acquire))
		throw std::logic_error("Gameplay reset is already in progress");
	if (m_resetFailed || m_impl->world.IsScheduledExecutionActive())
	{
		EndReset();
		throw std::logic_error("Cannot reset failed gameplay state or reset during scheduled execution");
	}
}
void GameplayState::EndReset() noexcept { m_resetting.clear(std::memory_order_release); }
void GameplayState::ReplaceEmptyWorld()
{
	if (Count() != 0) throw std::logic_error("Cannot reset gameplay while entities are still owned");
	if (m_worldBindings != 0)
		throw std::logic_error("Cannot replace gameplay World while execution/query bindings remain alive");
	auto replacement = std::make_unique<Impl>();
	delete m_impl;
	m_impl = replacement.release();
}
}
}
