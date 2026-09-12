module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.capture.systems.capture_upgrade_activation_system;
export import games.generalszh.gameplay.capture.definitions.capture_definition;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.upgrades.state.upgrade_status;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.rts.economy.components.resource_balance;

namespace generalszh::capture_activation_detail
{
inline bool EntityLess(const ecs::Entity left,const ecs::Entity right) noexcept
{
    return std::tie(left.index,left.generation)<std::tie(right.index,right.generation);
}

struct Grant
{
    ecs::Entity account{};
    std::uint32_t definition{};
};

inline bool GrantLess(const Grant &left,const Grant &right) noexcept
{
    if(left.account!=right.account) return EntityLess(left.account,right.account);
    return left.definition<right.definition;
}
}

export namespace generalszh::capture
{
// Applies the one supported player-scoped UnpauseSpecialPowerUpgrade binding
// to capture actors. Research records remain authoritative; the sorted vectors
// are bounded per-tick derived scratch and are rebuilt before chunk jobs.
class CaptureUpgradeActivationSystem
{
    using Unit=production::ProducedUnit;
    using Life=engine::gameplay::combat::LifeState;
    using Balance=engine::gameplay::rts::economy::ResourceBalance;
    using Grants=ecs::Query<ecs::Read<production::ResearchedUpgrade>,ecs::Read<upgrades::UpgradeInstanceStatus>>;
    using Accounts=ecs::Query<ecs::Read<Balance>>;
public:
    using Query=ecs::Query<ecs::Write<CaptureCapability>,ecs::Write<CaptureRecharge>,ecs::Read<Unit>,ecs::Read<Life>>;
    using AuxiliaryAccess=ecs::Query<ecs::Read<production::ResearchedUpgrade>,
        ecs::Read<upgrades::UpgradeInstanceStatus>,ecs::Read<Balance>>;

    CaptureUpgradeActivationSystem(ecs::World &world,const CaptureDefinitions &definitions,std::size_t capacity)
        :definitions_(definitions),grants_(world),accounts_(world),capacity_(capacity)
    {
        if(!capacity) throw std::invalid_argument("Capture activation requires positive entity capacity");
        validAccounts_.reserve(capacity);completedGrants_.reserve(capacity);
    }

    void BeforeChunks(Query &,ecs::SystemContext &)
    {
        validAccounts_.clear();completedGrants_.clear();
        accounts_.ForEachChunk([&](auto chunk) {
            if(chunk.Count()>capacity_-validAccounts_.size())
                throw std::length_error("Capture activation account capacity exhausted");
            for(const auto entity:chunk.Entities()) validAccounts_.push_back(entity);
        });
        std::sort(validAccounts_.begin(),validAccounts_.end(),capture_activation_detail::EntityLess);
        validAccounts_.erase(std::unique(validAccounts_.begin(),validAccounts_.end()),validAccounts_.end());

        grants_.ForEachChunk([&](auto chunk) {
            const auto values=chunk.template Get<production::ResearchedUpgrade>();
            const auto statuses=chunk.template Get<upgrades::UpgradeInstanceStatus>();
            if(chunk.Count()>capacity_-completedGrants_.size())
                throw std::length_error("Capture activation grant capacity exhausted");
            for(std::size_t row=0;row<chunk.Count();++row)
            {
                const auto &grant=values[row];
                if(statuses[row].value!=upgrades::Status::Complete
                    ||!std::binary_search(validAccounts_.begin(),validAccounts_.end(),grant.account,
                        capture_activation_detail::EntityLess)) continue;
                completedGrants_.push_back({grant.account,grant.definition});
            }
        });
        std::sort(completedGrants_.begin(),completedGrants_.end(),capture_activation_detail::GrantLess);
        completedGrants_.erase(std::unique(completedGrants_.begin(),completedGrants_.end(),[](const auto &left,const auto &right) {
            return left.account==right.account&&left.definition==right.definition;
        }),completedGrants_.end());
    }

    void Execute(Query::Chunk chunk,ecs::SystemContext &context) const
    {
        auto capabilities=chunk.Get<CaptureCapability>();
        auto recharges=chunk.Get<CaptureRecharge>();
        const auto units=chunk.Get<Unit>();
        const auto lives=chunk.Get<Life>();
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            // Death is an explicit lifecycle boundary. Do not enroll a dead
            // actor, and do not lazily repair an uninitialized recharge.
            if(!lives[row].alive||capabilities[row].enabled||!recharges[row].initialized) continue;
            const auto &definition=definitions_.GetUnchecked(capabilities[row].definition);
            if(!definition.activationUpgrade||!HasGrant(units[row].account,definition.activationUpgrade->definition)) continue;
            capabilities[row].enabled=true;
            // This is only the startup-paused first unlock. It mirrors the
            // legacy pauseCountdown(FALSE) deadline shift and its startsReady
            // setReadyFrame(now) override; runtime capture recharge is owned by
            // CaptureSystem and is not reset here.
            recharges[row].readyAt=CaptureDeadline(context.Tick(),definition.startsReady?0:definition.rechargeTicks);
        }
    }

private:
    bool HasGrant(const ecs::Entity account,const std::uint32_t definition) const noexcept
    {
        const auto found=std::lower_bound(completedGrants_.begin(),completedGrants_.end(),
            capture_activation_detail::Grant{account,definition},capture_activation_detail::GrantLess);
        return found!=completedGrants_.end()&&found->account==account&&found->definition==definition;
    }

    const CaptureDefinitions &definitions_;
    Grants grants_;
    Accounts accounts_;
    const std::size_t capacity_;
    std::vector<ecs::Entity> validAccounts_;
    std::vector<capture_activation_detail::Grant> completedGrants_;
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::capture::CaptureUpgradeActivationSystem>
{
    static constexpr std::string_view StableName="games.generalszh.capture.upgrade_activation";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>;
    using After=SystemTypeList<>;
};
}
