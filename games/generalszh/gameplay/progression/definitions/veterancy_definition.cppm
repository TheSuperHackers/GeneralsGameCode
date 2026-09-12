module;
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <span>
#include <vector>
export module games.generalszh.gameplay.progression.definitions.veterancy_definition;
export import engine.gameplay.progression.definitions.progression_definition;
export namespace generalszh::progression
{
enum class VeterancyLevel : std::uint32_t { Regular, Veteran, Elite, Heroic };
// GameCommon.h defines four levels; ThingTemplate defaults every entry to zero
// and IsTrainable to false. No universal unit thresholds exist.
struct VeterancyDefinition
{
 std::array<std::uint32_t,4> required{},awardValues{};
 bool trainable{};
 // Resolved player-rank awards are deliberately separate from unit XP
 // awardValues. The content adapter resolves the legacy -999 fallback once;
 // runtime kill attribution never reads the unit-XP stream for rank points.
 std::array<std::uint32_t,4> skillPointValues{};
 engine::gameplay::progression::ProgressionDefinition Progression() const
 {
  engine::gameplay::progression::ProgressionDefinition result;
  result.maximumExperience=2147483647u;
  for(auto value:required) {
   if(value>result.maximumExperience) throw std::invalid_argument("XP threshold exceeds Zero Hour integer range");
   result.thresholds.push_back(value);
  }
  if(required[0]!=0 || required[1]>required[2] || required[2]>required[3])
   throw std::invalid_argument("Zero Hour thresholds must be cumulative from zero");
 for(auto value:awardValues) if(value>result.maximumExperience)
   throw std::invalid_argument("XP award exceeds Zero Hour integer range");
  for(auto value:skillPointValues) if(value>result.maximumExperience)
   throw std::invalid_argument("Skill-point award exceeds Zero Hour integer range");
  return result;
 }
 std::uint32_t AwardValue(VeterancyLevel victimLevel) const
 { return awardValues.at(static_cast<std::size_t>(victimLevel)); }
 std::uint32_t SkillPointValue(VeterancyLevel victimLevel) const
 { return skillPointValues.at(static_cast<std::size_t>(victimLevel)); }
};

// Immutable Zero Hour award data retained alongside the generic progression
// thresholds. Runtime component references use an independently authored dense
// index; adapters map stable unit keys explicitly and never rely on generic
// progression catalog positions. This catalog never owns ECS state or execution.
class VeterancyCatalog
{
public:
 explicit VeterancyCatalog(std::span<const VeterancyDefinition> definitions) : definitions_(definitions.begin(),definitions.end())
 {
  for(const auto &definition:definitions_) (void)definition.Progression();
 }
 std::size_t Size() const noexcept { return definitions_.size(); }
 const VeterancyDefinition &Get(const std::size_t index) const
 {
  if(index>=definitions_.size()) throw std::out_of_range("Veterancy definition index out of range");
  return definitions_[index];
 }
private:
 std::vector<VeterancyDefinition> definitions_;
};
}
