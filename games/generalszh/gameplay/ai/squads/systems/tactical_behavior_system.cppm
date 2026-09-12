module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

export module games.generalszh.gameplay.ai.squads.systems.tactical_behavior_system;

export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.components.weapon;
export import engine.gameplay.containment.components.containment_task;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.rts.construction.components.construction_components;
export import engine.gameplay.rts.harvesting.components.harvest_state;
export import engine.gameplay.rts.repair.components.manual_repair;
export import engine.gameplay.rts.ai.squads.algorithms.squad_anchor;
export import engine.gameplay.rts.ai.squads.algorithms.squad_goal;
export import engine.gameplay.rts.ai.squads.algorithms.squad_order_decision;
export import engine.gameplay.rts.ai.squads.components.squad_goal;
export import engine.gameplay.rts.ai.squads.components.squad_goal_application;
export import engine.gameplay.rts.ai.squads.components.squad_membership;
export import engine.gameplay.rts.ai.squads.inputs.squad_order_request_batch;
export import engine.gameplay.concealment.components.concealment_state;
export import engine.gameplay.rts.visibility.components.visibility_cell_page;
export import games.generalszh.gameplay.combat.acquisition.acquisition_policy;
export import games.generalszh.gameplay.capture.components.capture_state;
export import games.generalszh.gameplay.combat.targeting.target_index;
export import games.generalszh.gameplay.production.runtime.production_runtime;

export namespace generalszh::ai::squads
{

using SquadMembership = engine::gameplay::rts::ai::squads::SquadMembership;
using SquadGoal = engine::gameplay::rts::ai::squads::SquadGoal;
using SquadGoalKind = engine::gameplay::rts::ai::squads::SquadGoalKind;
using SquadGoalApplication = engine::gameplay::rts::ai::squads::SquadGoalApplication;
using SquadAnchorCandidate = engine::gameplay::rts::ai::squads::SquadAnchorCandidate;
using SquadOrderRequest = engine::gameplay::rts::ai::squads::SquadOrderRequest;
using SquadOrderRequestBatch = engine::gameplay::rts::ai::squads::SquadOrderRequestBatch;

struct TacticalCapacity
{
	std::size_t maxSquads{};
	std::size_t maxMembers{};
	std::size_t maxPrimaryChunks{};
	std::size_t maxRequests{};
};

using TacticalBehaviorCapacity = TacticalCapacity;

class TacticalBehaviorSystem final
{
public:
	using Query = ecs::Query<
		ecs::Read<SquadMembership>,
		ecs::Write<SquadGoalApplication>,
		ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Read<generalszh::production::ProducedUnit>,
		ecs::Read<engine::gameplay::navigation::GridPosition>,
		ecs::Optional<engine::gameplay::combat::WeaponDefinition>,
		ecs::Optional<engine::gameplay::containment::PassengerMembership>,
		ecs::Optional<engine::gameplay::rts::construction::BuilderAssignment>,
		ecs::Optional<engine::gameplay::rts::harvesting::HarvestState>,
		ecs::Optional<engine::gameplay::containment::ContainmentTask>,
		ecs::Optional<generalszh::capture::CaptureActorState>,
		ecs::Optional<engine::gameplay::rts::repair::ManualRepairAssignment>>;

	// This is the exact TargetIndex::VisibilityAccess set plus the squad goal
	// column.  The separate goal query below supplies data; this alias supplies
	// complete scheduler conflict metadata for that caller-side preparation.
	using AuxiliaryAccess = ecs::Query<
		ecs::Read<SquadGoal>,
		ecs::Read<engine::gameplay::combat::Health>,
		ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Read<engine::gameplay::navigation::GridPosition>,
		ecs::Optional<generalszh::production::ProducedUnit>,
		ecs::Optional<generalszh::production::Producer>,
		ecs::Optional<generalszh::construction::Structure>,
		ecs::Optional<engine::gameplay::containment::PassengerMembership>,
		ecs::Optional<engine::gameplay::rts::construction::BuilderAssignment>,
		ecs::Optional<engine::gameplay::rts::harvesting::HarvestState>,
		ecs::Optional<engine::gameplay::containment::ContainmentTask>,
		ecs::Optional<generalszh::capture::CaptureActorState>,
		ecs::Optional<engine::gameplay::rts::repair::ManualRepairAssignment>,
		ecs::Optional<engine::gameplay::concealment::ConcealmentState>,
		ecs::Read<engine::gameplay::rts::visibility::VisibilityCellIndex>,
		ecs::Read<engine::gameplay::rts::visibility::VisibilityVisibleMaskPage>>;

	TacticalBehaviorSystem(ecs::World &world,
		const engine::gameplay::navigation::NavigationGrid &grid,
		generalszh::combat::TargetIndex &targetIndex,
		SquadOrderRequestBatch &requests,
		const TacticalCapacity capacity) :
		grid_(grid), targetIndex_(targetIndex), requests_(requests), capacity_(capacity),
		goals_(world)
	{
		if (capacity_.maxSquads == 0 || capacity_.maxMembers == 0 ||
			capacity_.maxPrimaryChunks == 0 || capacity_.maxRequests == 0)
			throw std::invalid_argument("Tactical squad capacities must be positive");
		if (capacity_.maxRequests > requests_.Capacity())
			throw std::invalid_argument("Tactical request capacity exceeds its injected batch");

		goalRows_.reserve(capacity_.maxSquads);
		memberRows_.reserve(capacity_.maxMembers);
		memberOrder_.reserve(capacity_.maxMembers);
		rowBases_.reserve(capacity_.maxPrimaryChunks);
		slots_.reserve(capacity_.maxMembers);
		requestsToPublish_.reserve(capacity_.maxRequests);
		anchorCandidates_.reserve(capacity_.maxMembers);
	}

	void BeforeChunks(Query &query, ecs::SystemContext &context)
	{
		requests_.Begin(context.Tick());
		targetIndex_.Rebuild(context.Tick());

		goalRows_.clear();
		goals_.ForEachChunk([&](auto chunk)
		{
			const auto values = chunk.template Get<SquadGoal>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				if (goalRows_.size() == capacity_.maxSquads)
					throw std::length_error("Tactical squad capacity exhausted");
				assert(engine::gameplay::rts::ai::squads::IsValidSquadGoal(values[row], grid_));
				goalRows_.push_back(GoalRow{chunk.Entities()[row], values[row]});
			}
		});
		std::sort(goalRows_.begin(), goalRows_.end(), [](const GoalRow &left, const GoalRow &right)
		{
			return EntityLess(left.squad, right.squad);
		});

		rowBases_.clear();
		memberRows_.clear();
		query.ForEachPreparedChunk([&](auto chunk)
		{
			if (rowBases_.size() == capacity_.maxPrimaryChunks)
				throw std::length_error("Tactical primary chunk capacity exhausted");
			if (chunk.Count() > capacity_.maxMembers - memberRows_.size())
				throw std::length_error("Tactical member capacity exhausted");
			rowBases_.push_back(memberRows_.size());

			const auto memberships = chunk.template Get<SquadMembership>();
			const auto lives = chunk.template Get<engine::gameplay::combat::LifeState>();
			const auto units = chunk.template Get<generalszh::production::ProducedUnit>();
			const auto positions = chunk.template Get<engine::gameplay::navigation::GridPosition>();
			const auto weapons = chunk.template Get<engine::gameplay::combat::WeaponDefinition>();
			const auto passengers = chunk.template Get<engine::gameplay::containment::PassengerMembership>();
			const auto builderAssignments = chunk.template Get<engine::gameplay::rts::construction::BuilderAssignment>();
			const auto harvestStates = chunk.template Get<engine::gameplay::rts::harvesting::HarvestState>();
			const auto containmentTasks = chunk.template Get<engine::gameplay::containment::ContainmentTask>();
			const auto captureStates = chunk.template Get<generalszh::capture::CaptureActorState>();
			const auto repairAssignments = chunk.template Get<engine::gameplay::rts::repair::ManualRepairAssignment>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto cell = positions[row].cell;
				const bool taskOwned =
					(!passengers.empty() && engine::gameplay::containment::IsContained(passengers[row])) ||
					(!builderAssignments.empty() && builderAssignments[row].site.IsValid()) ||
					(!harvestStates.empty() && harvestStates[row].phase != engine::gameplay::rts::harvesting::HarvestPhase::Idle &&
						harvestStates[row].phase != engine::gameplay::rts::harvesting::HarvestPhase::NeedsInput) ||
					(!containmentTasks.empty() && containmentTasks[row].carrier.IsValid()) ||
					(!captureStates.empty() && generalszh::capture::IsCaptureBusy(captureStates[row])) ||
					(!repairAssignments.empty() && repairAssignments[row].target.IsValid());
				memberRows_.push_back(MemberObservation{
					chunk.Entities()[row], memberships[row].squad, units[row].account, cell,
					lives[row].alive,
					!passengers.empty() && engine::gameplay::containment::IsContained(passengers[row]),
					cell < grid_.Count() && grid_.Walkable(cell),
					!weapons.empty() && weapons[row].rangeCells != 0,
					taskOwned});
			}
		});

		memberOrder_.resize(memberRows_.size());
		for (std::size_t index = 0; index != memberOrder_.size(); ++index)
			memberOrder_[index] = index;
		std::sort(memberOrder_.begin(), memberOrder_.end(), [&](const std::size_t left, const std::size_t right)
		{
			const auto &a = memberRows_[left];
			const auto &b = memberRows_[right];
			if (a.squad != b.squad)
				return EntityLess(a.squad, b.squad);
			return EntityLess(a.actor, b.actor);
		});

		slots_.assign(memberRows_.size(), DecisionSlot{});
		PrepareGoalTargets();
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &context) noexcept
	{
		assert(context.ChunkOrder() < rowBases_.size());
		const auto base = rowBases_[context.ChunkOrder()];
		const auto memberships = chunk.template Get<SquadMembership>();
		auto applications = chunk.template Get<SquadGoalApplication>();

		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			const auto slotIndex = base + row;
			assert(slotIndex < memberRows_.size());
			auto &slot = slots_[slotIndex];
			slot = {};

			const auto *goal = FindGoal(memberships[row].squad);
			if (goal == nullptr)
				continue; // a stale squad generation never changes the latch
			if (applications[row].squad == goal->squad &&
				applications[row].appliedRevision == goal->goal.revision)
				continue;

			// The matching revision is latched before eligibility is considered.
			// This is the control handoff rule that prevents a persistent goal from
			// reclaiming a later player-controlled member.
			applications[row] = {goal->squad, goal->goal.revision};

			const auto &member = memberRows_[slotIndex];
			if (member.taskOwned)
				continue;
			const auto target = ResolvedTarget(*goal);
			bool targetVisible = false;
			if (target.IsValid() && member.positionValid && !member.contained)
			{
				const auto &frame = targetIndex_.Frame();
				if (frame.Contains(member.actor) && frame.Contains(target))
					targetVisible = frame.CanTarget(member.actor, target);
			}

			const auto decision = engine::gameplay::rts::ai::squads::DecideSquadOrder(
				goal->goal,
				{member.actor, member.owner, member.alive, member.contained, member.positionValid,
					member.hasWeapon, target, targetVisible});
			if (!decision.emit)
				continue;
			slot = {true, goal->squad, goal->goal.revision, member.actor, decision.order};
		}
	}

	void AfterChunks(Query &, ecs::SystemContext &)
	{
		requestsToPublish_.clear();
		for (const auto &slot : slots_)
		{
			if (!slot.valid)
				continue;
			if (requestsToPublish_.size() == capacity_.maxRequests)
				throw std::length_error("Tactical order request capacity exhausted");
			requestsToPublish_.push_back({slot.squad, slot.revision, {slot.actor, slot.order}});
		}
		std::sort(requestsToPublish_.begin(), requestsToPublish_.end(),
			engine::gameplay::rts::ai::squads::SquadOrderRequestLess);
		for (const auto &request : requestsToPublish_)
			requests_.Append(request);
		requests_.Publish();
	}

private:
	struct GoalRow
	{
		ecs::Entity squad{};
		SquadGoal goal{};
		ecs::Entity anchorActor{};
		engine::gameplay::navigation::Cell anchor{engine::gameplay::navigation::InvalidCell};
		ecs::Entity resolvedTarget{};
	};

	struct MemberObservation
	{
		ecs::Entity actor{};
		ecs::Entity squad{};
		ecs::Entity owner{};
		engine::gameplay::navigation::Cell position{engine::gameplay::navigation::InvalidCell};
		bool alive{};
		bool contained{};
		bool positionValid{};
		bool hasWeapon{};
		bool taskOwned{};
	};

	struct DecisionSlot
	{
		bool valid{};
		ecs::Entity squad{};
		std::uint64_t revision{};
		ecs::Entity actor{};
		engine::gameplay::rts::orders::UnitOrder order{};
	};

	using GoalQuery = ecs::Query<ecs::Read<SquadGoal>>;

	static bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
	{
		return left.index < right.index ||
			(left.index == right.index && left.generation < right.generation);
	}

	const GoalRow *FindGoal(const ecs::Entity squad) const noexcept
	{
		const auto found = std::lower_bound(goalRows_.begin(), goalRows_.end(), squad,
			[](const GoalRow &row, const ecs::Entity value) { return EntityLess(row.squad, value); });
		return found == goalRows_.end() || found->squad != squad ? nullptr : &*found;
	}

	std::pair<std::size_t, std::size_t> MemberRange(const ecs::Entity squad) const
	{
		const auto first = std::lower_bound(memberOrder_.begin(), memberOrder_.end(), squad,
			[&](const std::size_t index, const ecs::Entity value)
			{ return EntityLess(memberRows_[index].squad, value); });
		const auto last = std::upper_bound(first, memberOrder_.end(), squad,
			[&](const ecs::Entity value, const std::size_t index)
			{ return EntityLess(value, memberRows_[index].squad); });
		return {static_cast<std::size_t>(first - memberOrder_.begin()),
			static_cast<std::size_t>(last - first)};
	}

	void PrepareGoalTargets()
	{
		const auto &frame = targetIndex_.Frame();
		for (auto &goal : goalRows_)
		{
			if (goal.goal.kind != SquadGoalKind::Hunt)
			{
				if (goal.goal.kind == SquadGoalKind::AttackTarget)
					goal.resolvedTarget = goal.goal.target;
				continue;
			}

			anchorCandidates_.clear();
			const auto [first, count] = MemberRange(goal.squad);
			for (std::size_t offset = 0; offset != count; ++offset)
			{
				const auto &member = memberRows_[memberOrder_[first + offset]];
				if (member.owner == goal.goal.owner && member.alive && !member.contained && member.positionValid)
					anchorCandidates_.push_back({member.actor, member.position});
			}
			const auto anchor = engine::gameplay::rts::ai::squads::SelectSquadAnchor(anchorCandidates_);
			if (!anchor)
				continue;
			goal.anchorActor = anchor->actor;
			goal.anchor = anchor->position;
			goal.resolvedTarget = frame.Nearest(goal.anchor, goal.goal.owner,
				goal.goal.searchRadiusCells, generalszh::combat::UnitTarget);
		}
	}

	static ecs::Entity ResolvedTarget(const GoalRow &goal) noexcept
	{
		return goal.goal.kind == SquadGoalKind::AttackTarget || goal.goal.kind == SquadGoalKind::Hunt
			? goal.resolvedTarget : ecs::Entity{};
	}

	const engine::gameplay::navigation::NavigationGrid &grid_;
	generalszh::combat::TargetIndex &targetIndex_;
	SquadOrderRequestBatch &requests_;
	const TacticalCapacity capacity_;
	GoalQuery goals_;
	std::vector<GoalRow> goalRows_;
	std::vector<MemberObservation> memberRows_;
	std::vector<std::size_t> memberOrder_;
	std::vector<std::size_t> rowBases_;
	std::vector<DecisionSlot> slots_;
	std::vector<SquadOrderRequest> requestsToPublish_;
	std::vector<SquadAnchorCandidate> anchorCandidates_;
};
} // namespace generalszh::ai::squads

export namespace ecs
{

template<>
struct SystemTraits<generalszh::ai::squads::TacticalBehaviorSystem>
{
	static constexpr std::string_view StableName = "games.generalszh.ai.squads.tactical_behavior";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};

} // namespace ecs
