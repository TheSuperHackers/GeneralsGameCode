module;
#include <cassert>
#include <cstdint>
export module engine.gameplay.rts.repair.algorithms.repair_progress;
export import engine.gameplay.rts.repair.definitions.repair_definition;
export namespace engine::gameplay::rts::repair
{
// Integer fractional accumulation, independent of health storage and game rules.
// Catalog is immutable while a remainder is live. No multiply of full capacity.
inline std::uint64_t RepairPulse(std::uint64_t capacity,const CompiledRepairDefinition &definition,std::uint32_t &remainder) noexcept
{
    assert(definition.denominator!=0 && definition.numerator<=definition.denominator);
    assert(remainder<definition.denominator);
    const auto denominator=definition.denominator;
    const auto whole=(capacity/denominator)*definition.numerator;
    const auto fractional=(capacity%denominator)*definition.numerator+remainder;
    remainder=static_cast<std::uint32_t>(fractional%denominator);
    return whole+fractional/denominator;
}
}
