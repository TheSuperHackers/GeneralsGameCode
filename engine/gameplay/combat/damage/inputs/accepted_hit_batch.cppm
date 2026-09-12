module;
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.combat.damage.inputs.accepted_hit_batch;
export import engine.gameplay.combat.damage.components.damage_packet;
export namespace engine::gameplay::combat::damage {
// Accepted and armor-resolved, NOT actual health loss or lethal attribution.
struct ResolvedHit {
    ecs::Entity target{},projectile{};
    std::uint64_t launchTick{},impactTick{};
    DamagePacket packet{};
    std::uint64_t resolvedQuantity{};
    bool saturated{};
};
// Single joined producer appends in canonical delivery order; consumers run
// after Publish. Root owns lifetime; no callbacks, per-hit allocations or replay.
class AcceptedHitBatch {
public:
    explicit AcceptedHitBatch(std::size_t capacity):capacity_(capacity) {hits_.reserve(capacity);}
    void BeginTick() {
        if(staging_) throw std::logic_error("Accepted hits already staging");
        published_=0;hits_.clear();staging_=true;
    }
    void Append(ResolvedHit hit) {
        if(!staging_) throw std::logic_error("Accepted hits require BeginTick");
        if(hits_.size()==capacity_) throw std::length_error("Accepted hit capacity exhausted");
        hits_.push_back(hit);
    }
    void Publish() {
        if(!staging_) throw std::logic_error("Accepted hits are not staging");
        published_=hits_.size();staging_=false;
    }
    std::span<const ResolvedHit> Results() const noexcept {return {hits_.data(),published_};}
    std::size_t Capacity() const noexcept {return capacity_;}
private:
    std::size_t capacity_,published_{};bool staging_{};std::vector<ResolvedHit> hits_;
};
}
