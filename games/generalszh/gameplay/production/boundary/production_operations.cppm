module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.production.boundary.production_operations;
export import games.generalszh.gameplay.production.admission.build_catalog;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import games.generalszh.gameplay.production.algorithms.prerequisite_presence;
export import games.generalszh.gameplay.production.algorithms.production_prerequisite_evaluation;
export import games.generalszh.gameplay.production.inputs.build_inputs;
export import engine.gameplay.rts.economy.components.resource_balance;
export import games.generalszh.gameplay.production.rally.components.rally_point;
export namespace generalszh::production
{
    using Work = engine::gameplay::rts::production::BuildWork;
    using Enabled = engine::gameplay::rts::production::BuildEnabled;
    using Ready = engine::gameplay::rts::production::BuildReady;
    using Quantity = engine::gameplay::rts::production::ProductionQuantity;
    using Queue = engine::gameplay::rts::production::ProductionQueue;
    using Member = engine::gameplay::rts::production::ProductionQueueMember;
    using Balance = engine::gameplay::rts::economy::ResourceBalance;
    using Health = engine::gameplay::combat::Health;
    using Life = engine::gameplay::combat::LifeState;
    using Position = engine::gameplay::navigation::GridPosition;
    using Weapon = engine::gameplay::combat::WeaponDefinition;
    using Orders = ecs::Query<ecs::Read<production::BuildOrder>, ecs::Read<Quantity>, ecs::Write<Member>>;
    using Producers = ecs::Query<ecs::Read<production::Producer>, ecs::Read<Queue>, ecs::Read<Life>>;
    using Grants = ecs::Query<ecs::Read<production::ResearchedUpgrade>>;

struct MemberPosition { ecs::Entity producer; std::uint32_t position; ecs::Entity entry; };
// These helpers mutate only the supplied world. Their scratch contains copied keys,
// never authoritative state, dependencies, systems, or an execution pipeline.
    inline void CollectOrders(Orders &orders, std::vector<ecs::Entity> &ordered, std::size_t capacity)
    {
        ordered.clear();
        orders.ForEachChunk([&](auto chunk) {
            if (chunk.Count() > capacity - ordered.size()) throw std::length_error("Production order capacity exhausted");
            ordered.insert(ordered.end(), chunk.Entities().begin(), chunk.Entities().end());
        });
        std::sort(ordered.begin(), ordered.end(), [](auto a, auto b) {
            return std::tie(a.index, a.generation) < std::tie(b.index, b.generation);
        });
    }
    inline BuildAcceptance Quote(ecs::World &world, const BuildCatalog &catalog, Orders &orders, Grants &grants,
        ecs::Entity producer, std::uint32_t key, std::span<const production::BuildOrder> pendingOrders,
        const PrerequisitePresence &presence)
    {
        const auto *actor = world.Get<production::Producer>(producer);
        const auto *life = world.Get<Life>(producer);
        const auto *queue = world.Get<Queue>(producer);
        if (!actor || !life || !life->alive || !actor->active || !queue || !world.Get<Position>(producer) ||
            !world.IsAlive(actor->account))
            return BuildAcceptance::InvalidProducer;
        const auto *balance = world.Get<Balance>(actor->account);
        if (!balance) return BuildAcceptance::InvalidProducer;
        const auto index = catalog.DefinitionIndex(key);
        if (index == catalog.Definitions().size() || catalog.Definitions()[index].key != key) return BuildAcceptance::UnknownDefinition;
        const auto &definition = catalog.Definitions()[index];
        if (catalog.HasPrerequisites())
        {
            const auto objectKeys = presence.Keys(actor->account);
            if (!SatisfiesObjectPrerequisites(catalog, index, objectKeys))
                return BuildAcceptance::PrerequisitesUnmet;
            const auto sciencePrerequisites = catalog.SciencePrerequisites(index);
            if (!sciencePrerequisites.empty())
            {
                const auto *unlockState = world.Get<engine::gameplay::rts::unlocks::UnlockState>(actor->account);
                if (!unlockState)
                    throw std::logic_error("Science production requires an authoritative account unlock state");
                if (!SatisfiesSciencePrerequisites(sciencePrerequisites, *unlockState))
                    return BuildAcceptance::PrerequisitesUnmet;
            }
        }
        if (queue->count >= actor->queueLimit) return BuildAcceptance::QueueFull;
        if (balance->quantity < definition.cost) return BuildAcceptance::InsufficientFunds;
        if (definition.kind == production::EntryKind::Upgrade)
        {
            bool found = false;
            orders.ForEachChunk([&](auto chunk) {
                const auto members = chunk.template Get<Member>();
                const auto values = chunk.template Get<production::BuildOrder>();
                for (std::size_t row = 0; row != chunk.Count(); ++row)
                    found |= members[row].queue.IsValid() && values[row].kind == production::EntryKind::Upgrade
                        && values[row].account == actor->account && values[row].definition == key;
            });
            grants.ForEachChunk([&](auto chunk) { for (const auto &grant : chunk.template Get<production::ResearchedUpgrade>())
                found |= grant.account == actor->account && grant.definition == key; });
            for (const auto &order : pendingOrders)
                found |= order.kind == production::EntryKind::Upgrade && order.account == actor->account && order.definition == key;
            if (found) return BuildAcceptance::DuplicateUpgrade;
        }
        return BuildAcceptance::Accepted;
    }
    inline const BuildDefinition &DefinitionAt(const BuildCatalog &catalog, std::size_t index)
    {
        if (index >= catalog.Definitions().size())
            throw std::out_of_range("Build catalog index is not valid");
        return catalog.Definitions()[index];
    }
    inline production::BuildOrder MakeOrder(const BuildCatalog &catalog, ecs::Entity producer,
        ecs::Entity account, engine::gameplay::navigation::Cell spawn, std::size_t index, std::uint32_t paid)
    {
        const auto &definition = DefinitionAt(catalog,index);
        return {producer, account, definition.key, paid,
            definition.kind, definition.health, spawn, definition.cellsPerSecond,
            catalog.Weapon(index), definition.acquisition, definition.maximumHealth, catalog.Poison(index), catalog.Harvest(index), definition.builder,
            catalog.Regeneration(index), catalog.Progression(index), catalog.Lifetime(index),
             definition.transportDefinition ? std::optional{engine::gameplay::containment::TransportBinding{*definition.transportDefinition}} : std::nullopt,
             definition.passenger,definition.capture,definition.armor,catalog.CaptureTiming(index),catalog.ManualRepair(index),definition.initialPayload,
             catalog.Stealth(index),catalog.Detector(index),catalog.Veterancy(index),definition.garrisonable,
             definition.visibility ? std::optional{definition.visibility->id} : std::nullopt,
             definition.cashBountyCost};
    }
    inline production::BuildOrder MakeOrder(ecs::World &world, const BuildCatalog &catalog, ecs::Entity producer, std::size_t index)
    {
        const auto *actor = world.Get<production::Producer>(producer);
        const auto *position = world.Get<Position>(producer);
        if (!actor || !position) throw std::logic_error("Producer order requires account and position");
        const auto &definition = DefinitionAt(catalog,index);
        return MakeOrder(catalog,producer,actor->account,position->cell,index,definition.cost);
    }
    inline production::BuildOrder MakePayloadOrder(const BuildCatalog &catalog, ecs::Entity carrier,
        ecs::Entity account, engine::gameplay::navigation::Cell spawn, std::uint32_t passengerDefinition, std::size_t index)
    {
        const auto &definition = DefinitionAt(catalog,index);
        if (definition.key != passengerDefinition)
            throw std::logic_error("Payload catalog key does not match its resolved index");
        return MakeOrder(catalog,carrier,account,spawn,index,0);
    }
    inline void Refund(ecs::World &world, const production::BuildOrder &order)
    {
        if (auto *balance = world.Get<Balance>(order.account))
            balance->quantity = static_cast<std::uint32_t>(balance->quantity + order.paid);
    }
    // O(1) removal marking. Deferred entities remain visible until wave commit,
    // but their invalid membership excludes them from compaction and admission.
    inline void MarkUnlinked(ecs::World &world, ecs::Entity entry)
    {
        auto *member = world.Get<Member>(entry);
        const auto producer = member->queue;
        assert(producer.IsValid());
        if (auto *queue = world.Get<Queue>(producer)) { assert(queue->count); --queue->count; }
        *member = {};
    }
    // One bounded scan/sort per removal batch, preserving FIFO across entity reuse.
    // Scratch keys are copied before sorting; ECS remains the authoritative owner.
    inline void CompactMembers(ecs::World &world, Orders &orders, std::vector<MemberPosition> &membersToCompact, std::size_t capacity)
    {
        membersToCompact.clear();
        orders.ForEachChunk([&](auto chunk) {
            const auto members = chunk.template Get<Member>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                if (members[row].queue.IsValid())
                {
                    if (membersToCompact.size() == capacity)
                        throw std::length_error("Production member capacity exhausted");
                    membersToCompact.push_back({members[row].queue, members[row].position, chunk.Entities()[row]});
                }
        });
        std::sort(membersToCompact.begin(), membersToCompact.end(), [](const auto &a, const auto &b) {
            return std::tie(a.producer.index, a.producer.generation, a.position, a.entry.index, a.entry.generation)
                < std::tie(b.producer.index, b.producer.generation, b.position, b.entry.index, b.entry.generation);
        });
        ecs::Entity producer{};
        std::uint32_t position = 0;
        for (const auto &member : membersToCompact)
        {
            if (member.producer != producer) { producer = member.producer; position = 0; }
            world.Get<Member>(member.entry)->position = position++;
        }
    }

// Caller owns one-shot startup and terminal failure state. Boundary operations
// cannot recover a failed session or configure/restart its scheduler.
    inline ecs::Entity CreateProducer(ecs::World &world, ProductionBoundaryState state, std::size_t capacity, ecs::Entity account, std::uint32_t queueLimit = 9, std::uint64_t health = 1000, engine::gameplay::navigation::Cell spawn = 0)
    {
        RequireProductionBoundary(world, state);
        if (!health || !world.IsAlive(account) || !world.Get<Balance>(account)) throw std::invalid_argument("Producer needs health and a live account");
        Producers producers(world); std::size_t count = 0;
        producers.ForEachChunk([&](auto chunk) { count += chunk.Count(); });
        if (count == capacity) throw std::length_error("Modern producer capacity exhausted");
        const auto producer = world.Create<Queue>();
        try
        {
            world.Add<production::Producer>(producer); *world.Get<production::Producer>(producer) = {account, queueLimit, true};
            world.Add<Health>(producer); world.Get<Health>(producer)->current = health;
            world.Add<engine::gameplay::combat::HealthCapacity>(producer);world.Get<engine::gameplay::combat::HealthCapacity>(producer)->maximum=health;
            world.Add<engine::gameplay::combat::PendingHealing>(producer);world.Add<engine::gameplay::combat::HealingResult>(producer);
            world.Add<Life>(producer); world.Add<engine::gameplay::combat::PendingDamage>(producer); world.Add<engine::gameplay::combat::DamageResult>(producer);
            world.Add<Position>(producer); world.Get<Position>(producer)->cell = spawn;
            world.Add<rally::RallyPoint>(producer);
        }
        catch (...) { world.Destroy(producer); throw; }
        return producer;
    }

    inline BuildReceipt EnqueueBuild(ecs::World &world, ProductionBoundaryState state, const BuildCatalog &catalog, std::size_t capacity, ecs::Entity producer, std::uint32_t definitionKey)
    {
        RequireProductionBoundary(world, state);
        Orders orders(world); Grants grants(world);
        PrerequisitePresence presence(world, capacity);
        if (catalog.HasPrerequisites()) presence.Rebuild();
        const auto status = Quote(world, catalog, orders, grants, producer, definitionKey, {}, presence);
        if (status != BuildAcceptance::Accepted) return {status};
        std::vector<ecs::Entity> ordered; ordered.reserve(capacity);
        CollectOrders(orders, ordered, capacity);
        if (ordered.size() == capacity) throw std::length_error("Modern production order capacity exhausted");
        const auto index = catalog.DefinitionIndex(definitionKey);
        const auto order = MakeOrder(world, catalog, producer, index);
        const auto position = world.Get<Queue>(producer)->count;
        const auto entry = world.Create<production::BuildOrder, Work, Enabled, Ready, Quantity, Member>();
        *world.Get<production::BuildOrder>(entry) = order;
        *world.Get<Work>(entry) = catalog.Requirement(index);
        *world.Get<Quantity>(entry) = {catalog.Definitions()[index].quantity, 0};
        *world.Get<Member>(entry) = {producer, position};
        ++world.Get<Queue>(producer)->count;
        world.Get<Balance>(order.account)->quantity -= order.paid;
        return {BuildAcceptance::Accepted, entry};
    }
    inline bool CancelBuild(ecs::World &world, ProductionBoundaryState state, std::size_t capacity, ecs::Entity producer, ecs::Entity entry)
    {
        RequireProductionBoundary(world, state);
        const auto *order = world.Get<production::BuildOrder>(entry);
        if (!order || order->producer != producer) return false;
        Orders orders(world); std::vector<MemberPosition> members; members.reserve(capacity);
        Refund(world, *order); MarkUnlinked(world, entry); CompactMembers(world, orders, members, capacity);
        world.Destroy(entry); return true;
    }
    inline void StopProducer(ecs::World &world, ProductionBoundaryState state, std::size_t capacity, ecs::Entity producer)
    {
        RequireProductionBoundary(world, state);
        auto *actor = world.Get<production::Producer>(producer);
        if (!actor) return;
        actor->active = false;
        Orders orders(world); std::vector<MemberPosition> members; members.reserve(capacity);
        std::vector<ecs::Entity> ordered; ordered.reserve(capacity);
        CollectOrders(orders, ordered, capacity);
        bool removed = false;
        for (const auto entry : ordered)
            if (const auto &order = *world.Get<production::BuildOrder>(entry); order.producer == producer)
            {
                Refund(world, order); MarkUnlinked(world, entry); world.Destroy(entry); removed = true;
            }
        if (removed) CompactMembers(world, orders, members, capacity);
    }

}
