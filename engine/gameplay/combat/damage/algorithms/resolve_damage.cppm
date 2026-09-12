module;
#include <cstdint>
#include <stdexcept>
export module engine.gameplay.combat.damage.algorithms.resolve_damage;
export import engine.gameplay.combat.damage.components.damage_packet;
namespace engine::gameplay::combat::damage::detail {
class UnsupportedDamage final:public std::invalid_argument {
public: UnsupportedDamage():std::invalid_argument("Damage semantics cannot execute as ordinary health damage") {} };
[[noreturn]] void ThrowUnsupported() {throw UnsupportedDamage{};}
}
export namespace engine::gameplay::combat::damage {
inline DamageResolution ResolveDamage(std::uint64_t raw,ArmorMultiplier coefficient,ArmorApplication policy) {
    if(policy==ArmorApplication::Bypass) return {raw,false};
    if(policy!=ArmorApplication::Scale) detail::ThrowUnsupported();
    constexpr std::uint64_t scale=1'000'000;
    const std::uint64_t multiplier=coefficient.millionths;
    const auto whole=raw/scale;
    // Remainder product fits uint64: < 1e6 * UINT32_MAX.
    const auto fraction=(raw%scale)*multiplier/scale;
    if(multiplier&&whole>(UINT64_MAX-fraction)/multiplier) return {UINT64_MAX,true};
    return {whole*multiplier+fraction,false};
}
}
