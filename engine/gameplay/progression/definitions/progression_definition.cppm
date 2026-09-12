module;
#include <algorithm>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.progression.definitions.progression_definition;
export namespace engine::gameplay::progression
{
struct ProgressionDefinition
{
 std::vector<std::uint64_t> thresholds;
 std::uint64_t maximumExperience{};
};
// Frozen at construction. Dense references require this same catalog ordering
// when restoring state; persistence traits alone do not supply a codec.
class ProgressionCatalog
{
public:
 explicit ProgressionCatalog(std::span<const ProgressionDefinition> definitions)
  : definitions_(definitions.begin(),definitions.end())
 {
  if(definitions_.size()>UINT32_MAX)
   throw std::invalid_argument("Invalid progression catalog size");
  for(const auto &definition:definitions_)
   if(definition.thresholds.empty() || definition.thresholds.size()>UINT32_MAX
    || definition.thresholds.front()!=0
    || !std::is_sorted(definition.thresholds.begin(),definition.thresholds.end())
    || definition.thresholds.back()>definition.maximumExperience)
    throw std::invalid_argument("Invalid progression thresholds or cap");
 }
 const ProgressionDefinition &Get(std::uint32_t index) const { return definitions_.at(index); }
 std::size_t Size() const noexcept { return definitions_.size(); }
private:
 std::vector<ProgressionDefinition> definitions_;
};
}
